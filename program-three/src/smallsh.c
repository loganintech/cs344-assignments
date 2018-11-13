#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <limits.h>
#include <pthread.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>

#define true 1
#define false 0
#define bool int

char prompt_string = ':';
const char *cmd_delim = " ";
bool foreground_only = false;
int last_status = 0;

void prompt_and_read(char *buffer);
int get_input_redirection(char *buffer[], int *buffer_length);
int get_output_redirection(char **buffer, int *buffer_length);
void handle_sig(int sig);

int main(int argc, char *argv[])
{
    signal(SIGINT, handle_sig);
    signal(SIGTSTP, handle_sig);
    int null_file = open("/dev/null", O_WRONLY);
    char cwd[2048];
    pid_t background_processes[512];
    int background_process_index = 0;
    if (getcwd(cwd, sizeof(cwd)) == NULL)
    {
        fprintf(stderr, "Couldn't read current working directory.");
        fflush(stderr);
    }
    char *command = malloc(sizeof(char) * 2048);
    char *tokenizer_buffer = malloc(sizeof(char) * 2048);

    while (true)
    {
        memset(command, '\0', sizeof(command));
        memset(tokenizer_buffer, '\0', sizeof(tokenizer_buffer));

        prompt_and_read(command);

        // printf("Input: '%s'\n", command); fflush(stdout);
        char *program_name = strtok_r(command, cmd_delim, &tokenizer_buffer);
        // printf("Program Name: '%s'\n", program_name); fflush(stdout);

        if(program_name == NULL) {
            continue;
        }

        if (strcmp(program_name, "exit") == 0)
        {
            exit(0);
        }
        else if (strcmp(program_name, "cd") == 0)
        {
            char *dir = (char*) strtok_r(NULL, cmd_delim, &tokenizer_buffer);

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
            printf("Last Exited Status: %d\n", last_status);
            fflush(stdout);
        }
        else if (program_name[0] != '#')
        {
            // printf("Getting PID\n"); fflush(stdout);
            pid_t parent = getpid();

            char *token;
            char *args[512];
            memset(args, '\0', 512);

            // printf("Memset the Args\n"); fflush(stdout);
            int arg_index = 0;
            args[arg_index++] = program_name;

            char pid_token[8];
            memset(pid_token, '\0', 8);

            // printf("Memset PID_token\n"); fflush(stdout);

            while (token = (char*) strtok_r(NULL, cmd_delim, &tokenizer_buffer))
            {
                if (strcmp(token, args[arg_index - 1]) == 0)
                {
                    continue;
                }
                else
                {
                    args[arg_index++] = token;
                }
            }

            pid_t child = fork();

            if (child == -1)
            {
                fprintf(stderr, "Failed to start file.");
                fflush(stderr);
            }
            else if (child > 0)
            {
                //In the parent
                if (foreground_only || args[arg_index - 1][0] != '&') {
                    waitpid(child, &last_status, 0);
                    if(!WIFEXITED(last_status)) {
                        printf("Process exited with signal: %d\n", WIFSIGNALED(last_status));
                    }
                }
                else {
                    background_processes[background_process_index++] = child;

                    printf("Backgrounded Process: %d\n", child); fflush(stdout);
                }
            }
            else
            {
                if (args[arg_index - 1][0] == '&') {
                    args[--arg_index] = NULL;
                }
                // printf("Program Name: '%s'\n", program_name);

                int input_descriptor = get_input_redirection(args, &arg_index);
                // printf("Arg Index: %d\n", arg_index); fflush(stdout);
                int output_descriptor = get_output_redirection(args, &arg_index);
                // printf("Arg Index: %d\n", arg_index); fflush(stdout);

                if (input_descriptor < 0 || output_descriptor < 0) {
                    exit(1);
                }

                dup2(input_descriptor, 0);
                dup2(output_descriptor, 1);

                int result = execvp(program_name, args);
                close(output_descriptor);
                close(input_descriptor);

                return result;
                // printf("Command ran: %d\n", result);
            }
        }

        for (int i = 0; i < background_process_index; i++)
        {

            pid_t result = waitpid(background_processes[i], &last_status, WNOHANG);
            if (result > 0)
            {

                if (!WIFEXITED(last_status))
                {
                    printf("Process %d exited with signal: %d\n", result, WIFSIGNALED(last_status));
                } else {
                    printf("Process %d completed with status %d\n", result, last_status);
                }
                fflush(stdout);
                for (int x = i; x < background_process_index - 1; x++)
                {
                    background_processes[x] = background_processes[x + 1];
                }

                background_processes[background_process_index - 2] = 0;
            }
        }
    }

    free(command);
    free(tokenizer_buffer);

    return 0;
}

int get_input_redirection(char **buffer, int *buffer_length)
{

    // printf("Buffer Len: %d\n", *buffer_length);
    fflush(stdout);

    int found = 0;
    for (int i = 0; i < (*buffer_length) - 1; i++)
    {
        // printf("Checking buffer: %s\n", buffer[i]);
        fflush(stdout);
        if (strcmp(buffer[i], "<") == 0)
        {
            found = i;
        }
    }

    int file_desc;

    if (found != 0)
    {
        file_desc = open(buffer[found + 1], O_RDONLY, 0667);

        if (file_desc < 0)
        {
            printf("Couldn't open file for reading.\n");
            fflush(stdout);
            last_status = 1;
            return -1;
        }
    }
    else
    {
        file_desc = open("/dev/null", O_RDONLY);
        return file_desc;
    }

    for (int i = found; i < (*buffer_length) - 2; i++)
    {

        buffer[i] = buffer[i + 2];
    }

    buffer[*buffer_length - 2] = NULL;
    buffer[*buffer_length - 1] = NULL;

    *buffer_length = *buffer_length - 2;
    return file_desc;
}
int get_output_redirection(char **buffer, int *buffer_length) {

    // printf("Buffer Len: %d\n", *buffer_length); fflush(stdout);

    int found = 0;
    for (int i = 0; i < (*buffer_length) - 1; i++)
    {
        // printf("Checking buffer: %s\n", buffer[i]); fflush(stdout);
        if (strcmp(buffer[i], ">") == 0)
        {
            found = i;
        }
    }

    int file_desc;

    if (found != 0)
    {
        file_desc = open(buffer[found + 1], O_WRONLY | O_CREAT, 0667);

        if (file_desc < 0)
        {
            printf("Couldn't open file for writing.\n");
            fflush(stdout);
            last_status = 1;
            return -1;
        }
    }
    else
    {
        //stdout
        return 1;
    }

    for (int i = found; i < (*buffer_length) - 2; i++)
    {

        buffer[i] = buffer[i + 2];
    }

    buffer[*buffer_length - 2] = NULL;
    buffer[*buffer_length - 1] = NULL;

    *buffer_length = *buffer_length - 2;
    return file_desc;
}

void prompt_and_read(char *buffer)
{

    printf("%c ", prompt_string);
    fflush(stdout);

    fgets(buffer, 2048, stdin);

    char* pid_pos = strstr(buffer, "$$");

    int pid_index = pid_pos - buffer;
    int buffer_len = strlen(buffer);

    if(pid_pos != NULL) {
        buffer[pid_index] = '\0';
        if (pid_index < buffer_len - 2) {
            char* after_pid_replace = malloc(sizeof(char) * 2048);
            memset(after_pid_replace, '\0', 2048);
            strcpy(after_pid_replace, buffer + pid_index + 2);
            pid_t a_pid = getpid();
            char* pid_str = malloc(sizeof(char) * 10);
            memset(pid_str, '\0', 10);
            sprintf(pid_str, "%d", a_pid);
            strcat(buffer, pid_str);
            strcat(buffer, after_pid_replace);
        }
    }

    char *newpos = strchr(buffer, '\n');
    if(newpos != NULL) {
        *newpos = '\0';
    }
}

void handle_sig(int sig) {
    if (sig == 2) {
        signal(SIGINT, handle_sig);
        printf("Caught SIGINT\n");
    }
    else if (sig == 20) {
        signal(SIGTSTP, handle_sig);
        if(!foreground_only) {
            printf("Entering foreground only mode.\n");
            foreground_only = true;
        } else {
            printf("Exiting foreground only mode.\n");
            foreground_only = false;
        }
    }
}
