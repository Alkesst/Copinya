/**
UNIX Shell Project

Sistemas Operativos
Grados I. Informatica, Computadores & Software
Dept. Arquitectura de Computadores - UMA

Some code adapted from "Fundamentos de Sistemas Operativos", Silberschatz et al.

To compile and run the program:
   $ gcc Shell_project.c job_control.c -o Shell
   $ ./Shell          
	(then type ^D to exit program)

**/

#include "job_control.h"   // remember to compile with module job_control.c 

#define MAX_LINE 256 /* 256 chars per line, per command, should be enough. */

// -----------------------------------------------------------------------
//                            MAIN          
// -----------------------------------------------------------------------

int main(void) {
	char inputBuffer[MAX_LINE]; /* buffer to hold the command entered */
	int background;             /* equals 1 if a command is followed by '&' */
	char *args[MAX_LINE/2];     /* command line (of 256) has max of 128 arguments */
	// probably useful variables:
	int pid_fork, pid_wait; /* pid for created and waited process */
	int status;             /* status returned by wait */
	enum status status_res; /* status processed by analyze_status() */
	int info;				/* info processed by analyze_status() */
	char* status_res_str;
	printf("Welcome to \033[31mCopinya\033[0m!\n\n");
	while (1)   /* Program terminates normally inside get_command() after ^D is typed*/
	{   		
		printf("%s@\033[31mcopinya\033[0m:\x1b[36m%s\033[32m$\x1b[0m", getenv("USER"),getenv("PWD"));
		fflush(stdout);
		get_command(inputBuffer, MAX_LINE, args, &background);  /* get next command */

		
		if(args[0]==NULL) continue;   // if empty command
        pid_fork = fork();
        if(pid_fork){
        	if(!background){
				waitpid(pid_fork,&status,0);
				 if(WEXITSTATUS(status) != 0){
    				printf("Error command not found. %s\n", args[0]);
    			} else {
    				status_res = analyze_status(status, &info);
    			if (status_res == 0){
    				status_res_str = "SUSPENDED";
    			} else if (status_res == 1){
    				status_res_str = "SIGNALED";
    			} else if (status_res == 2){
    				status_res_str = "EXITED";
    			}
    		printf("\nForeground pid: %d, command: %s, %s, info: %d\n",
    				pid_fork, args[0], status_res_str, info);
    	}
			} else {
				printf("Background running job... pid: %d, command: %s\n", pid_fork, args[0]);
			}
        } else {
			exit(execvp(args[0], args));
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
