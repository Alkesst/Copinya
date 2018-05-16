#include <string.h>
#include <unistd.h>

typedef struct InternalCommand_{
	char* name;
} ShellCommands; 

static const ShellCommands shell_commands[] ={ {"cd"} };

int isAShellOrder(char* command_name, char* args[]){
	int equals = 0, i = 0;
	size_t length = sizeof(shell_commands)/sizeof(shell_commands[0]);
	while(!equals && i < length){
		equals = (strcmp(command_name, shell_commands[i].name));
		i++;
	}
	if (!equals){
		chdir(args[1]);
	}
	return !equals;
}
