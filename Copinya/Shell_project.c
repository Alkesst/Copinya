/**
UNIX Shell Project. Copinya custom UNIX Shell © 2018.

Sistemas Operativos
Grados I. Informatica, Computadores & Software
Dept. Arquitectura de Computadores - UMA

Some code adapted from "Fundamentos de Sistemas Operativos", Silberschatz et al.

Author: Alejandro Garau Madrigal.
**/

#include "job_control.h"
#include <libgen.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#define MAX_LINE 256 /* 256 chars per line, per command, should be enough. */
#define COPINYA "\033[31mcopinya\033[0m"

job* job_list;


void print_shell(void);
void SIGCHLD_handler(int);
int isAShellOrder(char* command_name, char* argsv[]);
pid_t execute_command(char* args[], pid_t process_gr, int input, int output, int isBackground, int pa_cerrar);


// -----------------------------------------------------------------------
//                            MAIN          
// -----------------------------------------------------------------------

int main(void) {
    ignore_terminal_signals();  // Ignores SIGINT SIGQUIT SIGTSTP SIGTTIN SIGTTOU signals.
    new_list("Copinya Jobs", job_list);
    char inputBuffer[MAX_LINE]; /* buffer to hold the command entered */
    // probably useful variables:
    int pid_fork, pid_wait;     /* pid for created and waited process */
    int status;                 /* status returned by wait */
    enum status status_res;     /* status processed by analyze_status() */
    int info;       /* info processed by analyze_status() */
    char* status_res_str; // Auxiliar definitions.
    struct output_commands output[25];
    size_t output_commands_length;
    new_process_group(getpid()); // PROCESS GROUP TAREA 2
    print_shell();
    signal(SIGCHLD, SIGCHLD_handler);
    while (1){   /* Program terminates normally inside get_command() after ^D is typed*/
        {
            //Obtiene el directorio actual y el nombre de usuario
            //Queda bonito
            char tstr[255]; getcwd(tstr, 255);
            printf("%s@%s:%s$ ",getlogin() , COPINYA, basename(tstr));
            fflush(stdout);
        }  
        //Shows in the shell the current user, the copinya shell and the current dir.
        block_SIGCHLD();
        get_command(inputBuffer, MAX_LINE-1, output, &output_commands_length);  /* get next command */
        if(output_commands_length == 0){ 
            unblock_SIGCHLD();
            continue;   // if empty csommand
        }
        if(!isAShellOrder(output[0].args[0], output[0].args)){
            int input = 0, output_ = 0;
            if(output[0].input_name != NULL){
                int temp = open(output[0].input_name, O_RDONLY);
                if(temp == -1) {
                    printf("Could not read from file: %s -: %s\n", output[0].input_name, strerror(errno));
                    unblock_SIGCHLD();
                    continue;
                }
                input = temp;
            }
             if(output[output_commands_length - 1].output_name != NULL){
                int temp = open(output[output_commands_length - 1].output_name, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                // 0644 chmod permissions !!!!!! lol IMPORTANTE BRONS
                if(temp == -1) {
                    printf("Could not write in file: %s -: %s\n", output[output_commands_length - 1].output_name, strerror(errno));
                    unblock_SIGCHLD();
                    continue;
                }
                output_ = temp;
            }
            if(output_commands_length == 1){
                pid_fork = execute_command(output[0].args, 0, input, output_, output[0].background, 0);
                if(pid_fork){
                    // FATHER.
                    if(!output[0].background){ // Checks if the command eecuted is a background command.
                        unblock_SIGCHLD();
                        set_terminal(pid_fork);
                        pid_wait = waitpid(pid_fork, &status, WUNTRACED);                               
                        set_terminal(getpid());
                    }
                    // STATUS MESSAGES THAT THE SHELL SENDS TO THE USER.
                    status_res = analyze_status(status, &info);
                    status_res_str = status_strings[status_res];
                    if(WEXITSTATUS(status) == 127){ 
                        printf("Error command not found: %s\n", output[0].args[0]);
                    } else if (output[0].background){
                        // block_SIGCHLD();
                        if(output[0].respawnable){
                            add_job(job_list, new_job(pid_fork, (const char**) output[0].args, RESPAWNABLE));
                        } else {
                            add_job(job_list, new_job(pid_fork, (const char**) output[0].args, BACKGROUND));
                        }
                        print_job_list(job_list);
                        // unblock_SIGCHLD();
                        printf("Background running job... pid: %d, command: %s\n", pid_fork, output[0].args[0]);
                    } else if (status_res == SUSPENDED){
                        printf("\nForeground pid: %d, command: %s, has been suspended...\n", pid_fork, output[0].args[0]);
                        add_job(job_list, new_job(pid_fork, (const char**) output[0].args, STOPPED));
                        set_terminal(getpid());
                    } else {
                        printf("\nForeground pid: %d, command: %s, %s, info: %d\n",     
                            pid_fork, output[0].args[0], status_res_str, info);                       
                    }
                } 
            } else {
                int pope[2]; // mess with the pope get a noscope
                pipe(pope);
                pid_fork = execute_command(output[0].args, 0, input, pope[1], 0, pope[0]);
                execute_command(output[1].args, pid_fork, pope[0], output_, 0, pope[1]);
                {
                    siginfo_t statusinfo;
                    unblock_SIGCHLD();
                    set_terminal(pid_fork);
                    // while ((pid_wait = wait(&status)) > 0);                      
                    // pid_wait = waitpid(pid_fork, &status, WUNTRACED);                               
                    pid_wait = waitid(P_PGID, pid_fork, &statusinfo, WUNTRACED);         
                    pid_wait = waitid(P_PGID, pid_fork, &statusinfo, WUNTRACED);         
                    //int waitid(idtype_t idtype, id_t id, siginfo_t *infop, int options);                      
                    set_terminal(getpid());
                    status_res = analyze_status(statusinfo.si_status, &info);
                    status_res_str = status_strings[status_res];
                    if (status_res == SUSPENDED){
                        printf("\nForeground pid: %d, command: %s, has been suspended...\n", pid_fork, output[0].args[0]);
                        add_job(job_list, new_job(pid_fork, (const char**) output[0].args, STOPPED));
                        set_terminal(getpid());
                    } else {
                        printf("\nForeground pid: %d, command: %s, %s, info: %d\n",     
                            pid_fork, output[0].args[0], status_res_str, info);                       
                    }
                }
            }
            unblock_SIGCHLD();
        } else {
            unblock_SIGCHLD();
        }
        /* the steps are:
             (1) fork a child process using fork()
             (2) the child process will invoke execvp()
             (3) if background == 0, the parent will wait, otherwise continue 
             (4) Shell shows a status message for processed command 
             (5) loop returns to get_commnad() function
        */

    } // end while
}

void print_shell(){
        printf("\n"
"        .-\"; ! ;\"-.\n"
"      .'!  : | :  !`.\n"
"     /\\  ! : ! : !  /\\ \n"
"    /\\ |  ! :|: !  | /\\ \n"
"   (  \\ \\ ; :!: ; / /  )\n"
"  ( `. \\ | !:|:! | / .' )      Welcome to\n"
"  (`. \\ \\ \\!:|:!/ / / .')      \033[31mcopinya\033[0m shell!\n"
"   \\ `.`.\\ |!|! |/,'.' /\n"
"    `._`.\\\\!!!// .'_.'\n"
"       `.`.\\|//.'.'\n"
"        |`._`n'_.'|  \n"
"        \"----^----\"\n\n");
}

void SIGCHLD_handler(int s){
    int exitStatus, info, hasToBeDeleted, n = 0;
    job* job = job_list->next;
    enum status status_res;
    pid_t pid;
    while(job != NULL){
        hasToBeDeleted = 0;
        pid = waitpid(job->pgid, &exitStatus, WNOHANG | WUNTRACED | WCONTINUED);
        if (pid == job->pgid){
            status_res = analyze_status(exitStatus, &info);
            if(status_res == EXITED || status_res == SIGNALED) {
                if(job->state == RESPAWNABLE){
                    if(WEXITSTATUS(exitStatus) != 127){
                        printf("Respawned process [%d]\n", n);
                        pid_t pid = fork();
                        if(pid){
                            job->pgid = pid;
                        } else {
                            unblock_SIGCHLD();
                            new_process_group(getpid());
                            restore_terminal_signals();
                            execvp(job->argsv[0], job->argsv);
                            exit(127);
                        }
                    } else {
                        hasToBeDeleted = 1;
                        printf("Respawnable closed unexpectedly...\n");
                    }
                } else {
                    hasToBeDeleted = 1;
                    printf(" [%d]+ Done\n", n);
                }
            } else if(job->state != STOPPED && status_res == SUSPENDED) {
                //Si se ha suspendido, hay que anotarlo y notificarlo
                job->state = STOPPED;
                printf(" - %d %s has been suspended\n", job->pgid, job->command);
            } else if(job->state == STOPPED && status_res == CONTINUED) {
                //Si se ha suspendido, hay que anotarlo y notificarlo
                job->state = BACKGROUND;
                printf(" - %d %s has been continued\n", job->pgid, job->command);
            }
        }
        if(hasToBeDeleted) {
            struct job_* jobToBeDeleted = job;
            job = job->next;
            delete_job(job_list, jobToBeDeleted);
        } else {
            job = job->next;
        }
        n++;
    }

}

pid_t execute_command(char* args[], pid_t process_gr, int input, int output, int isBackground, int pa_cerrar) {
    pid_t pid_fork = fork();
    if(!pid_fork){
        unblock_SIGCHLD();
        if(output != 0){
            dup2(output, fileno(stdout));
            if(pa_cerrar != 0) close(pa_cerrar);
        }
        if(input != 0){
            dup2(input, fileno(stdin));
            if(pa_cerrar != 0) close(pa_cerrar);
        }
        //Restores the signals here because this is the forked process.
        //The father is immune to the signals, this code does not
        //Modify the father behaviour.
        /* FOREGROUND COMMANDS. SON. */
        if (!process_gr){
            process_gr = getpid();
            new_process_group(process_gr);
            if(!isBackground){
                set_terminal(process_gr);
            }
        } else {
            setpgid(getpid(), process_gr);
        }
        restore_terminal_signals();
        execvp(args[0], args);
        exit(127);
    }
    return pid_fork;
}
