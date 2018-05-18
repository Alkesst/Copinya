#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include "job_control.h"

typedef struct InternalCommand_{
	char* name;
	void (*func)(int args, char* argsv[]);
} ShellCommands; 

extern job* job_list;
static void cd(int args, char* argsv[]);
static void jobs(int args, char* argsv[]);
static void fg(int args, char* argsv[]);
static void bg(int args, char* argsv[]);
static int args(char* argsv[]);
static const ShellCommands shell_commands[] ={ {"cd", cd}, {"jobs", jobs}, {"bg", bg}, {"fg", fg} };

int isAShellOrder(char* command_name, char* argsv[]){
	int equals = 1, i = 0;
	size_t length = sizeof(shell_commands)/sizeof(shell_commands[0]);
	while(equals && i < length){
		equals = (strcmp(command_name, shell_commands[i].name));
		i++;
	}
	if (!equals){
		shell_commands[i - 1].func(args(argsv), argsv);
	}
	return !equals;
}


void cd(int args, char* argsv[]){
	// cd a un directorio.
	if(chdir(argsv[1]) == -1){
		perror(argsv[1]);
	}
}

void jobs(int args, char* argsv[]){
	print_job_list(job_list);
}

static int args(char* argsv[]){
	// cuenta la cantidad de argumentos que tiene el array.
	int args_ = 0;
	while(argsv[args_] != NULL){
		args_++;
	}
	return args_;
}

static void bg(int args, char* argsv[]){
	 block_SIGCHLD();
    int num;

    if(args != 1) {
        num = atoi(argsv[1]);
    } else {
        num = 1;
    }
    job* job = get_item_bypos(job_list, num);
    if(job != NULL) {
        job->state = BACKGROUND;    // MODIFICAMOS EL ESTADO DEL JOB A BACKGROUND
        killpg(job->pgid, SIGCONT); // AL GRUPO DE PROCESOS job-pgid se le manda la señal SIGCONT
    } else {
        printf("Job %d doesn't exists\n", num);
    }

    unblock_SIGCHLD();
}

static void fg(int args, char* argsv[]){
	unblock_SIGCHLD();
    int num;

    if(args != 1) {
        num = atoi(argsv[1]);
    } else {
        num = 1;
    }

    job* job = get_item_bypos(job_list, num);
    if(job != NULL){
    	int status, info;
        enum status status_res;

        job->state = FOREGROUND;
        //Cedemos la terminal al grupo de procesos...
        set_terminal(job->pgid);
        //...y les hacemos notificar de ello para que puedan
        //ejecutarse sin problemas
        printf("%s \n", job->command);
        killpg(job->pgid, SIGCONT);
        unblock_SIGCHLD();
        pid_t p = waitpid(job->pgid, &status, WUNTRACED);
        set_terminal(getpid());
        
        block_SIGCHLD();
#ifdef __APPLE__
        if(waitpid(p, &num, WUNTRACED | WNOHANG) == -1) return; //Evita un crash en OS X
#endif
        status_res = analyze_status(status, &info);
        if(status_res == SUSPENDED) {
            //Si la tarea ha sido suspendida otra vez, lo hacemos notificar
            //al usuario y cambiamos el estado del trabajo
            printf("\n%s supsended with code %d\n", job->command, info);
            job->state = STOPPED;
        } else {
            //Si se ha cerrado el proceso, se lo hacemos saber al usuario
            //y eliminamos el trabajo de la lista
            printf("%s exited with status %s %d\n", job->command, status_strings[status_res], info);
            delete_job(job_list, job);
        }
    } else {
        printf("Job %d doesn't exists\n", num);
    }
    unblock_SIGCHLD();
}	
