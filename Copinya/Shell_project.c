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

#define MAX_LINE 256 /* 256 chars per line, per command, should be enough. */
#define COPINYA "\033[31mcopinya\033[0m"


char* status_boi(int status, int info);
void print_shell();

// -----------------------------------------------------------------------
//                            MAIN          
// -----------------------------------------------------------------------

int main(void) {
	ignore_terminal_signals();  // Ignores SIGINT SIGQUIT SIGTSTP SIGTTIN SIGTTOU signals.
	char inputBuffer[MAX_LINE]; /* buffer to hold the command entered */
	int background;             /* equals 1 if a command is followed by '&' */
	char *args[MAX_LINE/2];     /* command line (of 256) has max of 128 arguments */
	// probably useful variables:
	int pid_fork, pid_wait; 	/* pid for created and waited process */
	int status;             	/* status returned by wait */
	enum status status_res; 	/* status processed by analyze_status() */
	int info, gpid_father;		/* info processed by analyze_status() */
	char* status_res_str, *aux; // Auxiliar definitions.
	new_process_group(getpid()); // PROCESS GROUP TAREA 2
	print_shell();
	while (1){   /* Program terminates normally inside get_command() after ^D is typed*/   		
		aux = strdup(getenv("PWD"));
		//Shows in the shell the current user, the copinya shell and the current dir. 
		printf("%s@%s:%s$ ", getenv("USER"), COPINYA, basename(aux));
		free(aux);
		fflush(stdout);
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
				}
				// STATUS MESSAGES THAT THE SHELL SENDS TO THE USER.
				if(WEXITSTATUS(status) != 0){ 
					printf("Error command not found. %s\n", args[0]);
				} else if (background){
					printf("Background running job... pid: %d, command: %s\n", pid_fork, args[0]);
					status_res_str = status_boi(status, info);
				} else {
					status_res_str = status_boi(status, info);
					printf("\nForeground pid: %d, command: %s, %s, info: %d\n",		
	     				pid_fork, args[0], status_res_str, info);
				}
			} else{
				//Restores the signals here because this is the forked process.
				//The father is immune to the signals, this code does not
				//Modify the father behaviour.
				/* FOREGROUND COMMANDS. SON. */
				set_terminal(getpid());
				new_process_group(getpid());
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

char* status_boi(int status, int info){
	char* status_res_str;
	int status_res;
	status_res = analyze_status(status, &info);
	if (status_res == 0){
		status_res_str = "SUSPENDED";
	} else if (status_res == 1){
		status_res_str = "SIGNALED";
	} else if (status_res == 2){
		status_res_str = "EXITED";
	} else if (status_res == 3){
		status_res_str = "CONTINUED";
	}
    return status_res_str;
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
