#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <limits.h>
#include <pthread.h>
#include <pthread.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>

#define true 1
#define false 0
#define bool int

char prompt_string = ':';
const char *cmd_delim = " ";

void prompt_and_read(char *buffer);

int main(int argc, char *argv[])
{

    char cwd[2048];
    if (getcwd(cwd, sizeof(cwd)) == NULL)
    {
        fprintf(stderr, "Couldn't read current working directory.");
        fflush(stderr);
    }

    char *command          = malloc(sizeof(char) * 2048);
    char *tokenizer_buffer = malloc(sizeof(char) * 2048);

    while (true)
    {
        memset(command, '\0', sizeof(command));
        memset(tokenizer_buffer, '\0', sizeof(tokenizer_buffer));

        prompt_and_read(command);

        char *program_name = strtok_r(command, cmd_delim, &tokenizer_buffer);

        if (strcmp(program_name, "exit") == 0)
        {
            exit(0);
        }
        else if (strcmp(program_name, "cd") == 0)
        {
            char *dir = strtok_r(NULL, cmd_delim, &tokenizer_buffer);

            if (dir == NULL)
            {
                char *home_dir = getenv("HOME");
                chdir(home_dir);
                strcpy(cwd, home_dir);
            }
            else if (dir[0] == '/')
            {
                chdir(dir);
                strcpy(cwd, dir);
            }
            else
            {
                if (strcmp(dir, "..") == 0)
                {
                    for (int i = strlen(cwd) - 1; i >= 0; i--)
                    {
                        if (cwd[i] == '/')
                        {
                            cwd[i] = '\0';
                            break;
                        }
                    }
                }
                else if (strcmp(dir, ".") != 0)
                {
                    strcat(cwd, "/");
                    strcat(cwd, dir);
                }
                chdir(cwd);
            }
        }
        else if (strcmp(program_name, "status") == 0)
        {
            printf("CWD: %s\n", cwd);
            fflush(stdout);
        }
	else if (program_name[0] == '#') {}
        else
        {
            pid_t parent = getpid();
            pid_t child = fork();
            char *token;
            char *args[512];
            int arg_index = 0;
            args[arg_index++] = program_name;
            while (token = strtok_r(NULL, cmd_delim, &tokenizer_buffer))
	    {
	        if (token == NULL) break;
                if (strcmp(token, "&&") == 0) {
                    sprintf(token, "%d", parent);
                }
		if (strcmp(token, args[arg_index - 1]) == 0) break;
		printf("Adding token: '%s'\n", token); fflush(stdout);
		args[arg_index++] = token;
            }

            if (child == -1)
            {
                fprintf(stderr, "Failed to start file.");
                fflush(stderr);
            }
            else if (child > 0)
            {
                int status;
                waitpid(child, &status, 0);
            }
            else
            {
                //In the child
                int result = execvp(program_name, args);
                printf("Command ran: %d\n", result);
            }
        }
    }

    free(command);
    free(tokenizer_buffer);

    return 0;
}

void prompt_and_read(char *buffer)
{

    printf("%c ", prompt_string);
    fflush(stdout);

    fgets(buffer, 2048, stdin);

    char *newpos = strchr(buffer, '\n');
    *newpos = '\0';
}
