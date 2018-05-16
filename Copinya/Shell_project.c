/**
UNIX Shell Project. Copinya custom UNIX Shell.

Sistemas Operativos
Grados I. Informatica, Computadores & Software
Dept. Arquitectura de Computadores - UMA

Some code adapted from "Fundamentos de Sistemas Operativos", Silberschatz et al.

Author: Alejandro Garau Madrigal.

To compile and run the program:
   $ gcc Shell_project.c job_control.c -o Shell
   $ ./Shell          
	(then type ^D to exit program)

**/

#include "job_control.h"   // remember to compile with module job_control.c 
#include "InternalCommands.c"
#include <libgen.h>
#include <string.h>
#include <signal.h>

#define MAX_LINE 256 /* 256 chars per line, per command, should be enough. */
#define COPINYA "\033[31mcopinya\033[0m"

job* job_list;


void print_shell();
void SIGCHLD_handler();

// -----------------------------------------------------------------------
//                            MAIN          
// -----------------------------------------------------------------------

int main(void) {
	ignore_terminal_signals();  // Ignores SIGINT SIGQUIT SIGTSTP SIGTTIN SIGTTOU signals.
	job_list = new_list("Copinya Jobs");
	char inputBuffer[MAX_LINE]; /* buffer to hold the command entered */
	int background;             /* equals 1 if a command is followed by '&' */
	char *args[MAX_LINE/2];     /* command line (of 256) has max of 128 arguments */
	// probably useful variables:
	int pid_fork, pid_wait; 	/* pid for created and waited process */
	int status;             	/* status returned by wait */
	enum status status_res; 	/* status processed by analyze_status() */
	int info, gpid_father;		/* info processed by analyze_status() */
	char* status_res_str; // Auxiliar definitions.
	new_process_group(getpid()); // PROCESS GROUP TAREA 2
	print_shell();
	while (1){   /* Program terminates normally inside get_command() after ^D is typed*/
		{
            //Obtiene el directorio actual y el nombre de usuario
            //Queda bonito
            char tstr[255]; getcwd(tstr, 255);
            printf("%s@%s:%s$ ",getlogin() , COPINYA, basename(tstr));
            fflush(stdout);
        }  
		//Shows in the shell the current user, the copinya shell and the current dir.
		get_command(inputBuffer, MAX_LINE, args, &background);  /* get next command */
		if(args[0]==NULL) continue;   // if empty command
		if(!isAShellOrder(args[0], args)){
			pid_fork = fork();
			if(pid_fork){
				// FATHER.
				if(!background){ // Checks if the command eecuted is a background command.
					set_terminal(pid_fork);
					pid_wait = waitpid(pid_fork, &status, WUNTRACED);								
					set_terminal(getpid());
				} else {
					block_SIGCHLD();
					add_job(job_list, new_job(pid_fork, args[0], BACKGROUND));
					print_job_list(job_list);
					unblock_SIGCHLD();
				}
				// STATUS MESSAGES THAT THE SHELL SENDS TO THE USER.
				if(WEXITSTATUS(status) != 0){ 
					printf("Error command not found: %s\n", args[0]);
				} else if (background){
					printf("Background running job... pid: %d, command: %s\n", pid_fork, args[0]);
					status_res_str = status_strings[analyze_status(status, &info)];
				} else {
					status_res_str = status_strings[analyze_status(status, &info)];
					printf("\nForeground pid: %d, command: %s, %s, info: %d\n",		
	     				pid_fork, args[0], status_res_str, info);
				}
			} else{
				//Restores the signals here because this is the forked process.
				//The father is immune to the signals, this code does not
				//Modify the father behaviour.
				/* FOREGROUND COMMANDS. SON. */
				new_process_group(getpid());
				if(!background){
					set_terminal(getpid());
				}
				restore_terminal_signals();
				exit(execvp(args[0], args));
			}
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

void SIGCHLD_handler(){
		
}
