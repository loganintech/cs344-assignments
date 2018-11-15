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

//A real bool would be nice
#define true 1
#define false 0
#define bool int

//Global variables are scary
char prompt_string = ':';
const char *cmd_delim = " ";
bool foreground_only = false;
int last_status = 0;

//Function signatures
void prompt_and_read(char *buffer);
int get_input_redirection(char *buffer[], int *buffer_length);
int get_output_redirection(char **buffer, int *buffer_length);
void handle_sig(int sig);

int main(int argc, char *argv[])
{
    //Setup to handle the signals the first time
    signal(SIGINT, handle_sig);
    signal(SIGTSTP, handle_sig);

    //Give some space for our data
    char cwd[2048];
    pid_t background_processes[512];
    int background_process_index = 0;
    char *command = malloc(sizeof(char) * 2048);
    char *tokenizer_buffer = malloc(sizeof(char) * 2048);

    //Grab the working directory
    if (getcwd(cwd, sizeof(cwd)) == NULL)
    {
        fprintf(stderr, "Couldn't read current working directory.");
        fflush(stderr);
    }

    while (true)
    {
        //For every background process
        for (int i = 0; i < background_process_index; i++)
        {
            //See if it exited
            pid_t result = waitpid(background_processes[i], &last_status, WNOHANG);
            //If it did
            if (result > 0)
            {

                //And it's a signal
                if (!WIFEXITED(last_status))
                {
                    //Print that out
                    fprintf(stdout, "Process %d exited with signal: %d\n", result, WTERMSIG(last_status));
                }
                else
                {
                    //Or just print exit status
                    fprintf(stdout, "Process %d completed with status: %d\n", result, WEXITSTATUS(last_status));
                }
                //Flush
                fflush(stdout);

                //And since it exited, remove it from our array
                for (int x = i; x < background_process_index - 1; x++)
                {
                    background_processes[x] = background_processes[x + 1];
                }

                //Set the last pointer to 0
                background_processes[background_process_index - 2] = 0;
            }
        }

        //Clear our our memory
        memset(command, '\0', sizeof(command));
        memset(tokenizer_buffer, '\0', sizeof(tokenizer_buffer));

        //Get our command
        prompt_and_read(command);

        char *program_name = strtok_r(command, cmd_delim, &tokenizer_buffer);

        //If it's null, go to the next loop
        if (program_name == NULL)
        {
            continue;
        }

        //If it's exit, exit
        if (strcmp(program_name, "exit") == 0)
        {
            exit(0);
        }
        //If it's CD, change dirs
        else if (strcmp(program_name, "cd") == 0)
        {
            char *dir = (char *)strtok_r(NULL, cmd_delim, &tokenizer_buffer);

            //If they typed no param go to home
            if (dir == NULL)
            {
                char *home_dir = getenv("HOME");
                chdir(home_dir);
                strcpy(cwd, home_dir);
            }
            //If it's an absolute path
            else if (dir[0] == '/')
            {
                chdir(dir);
                strcpy(cwd, dir);
            }
            //If it's a relative path
            else
            {
                //If it's asking for the parent
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
                //If it's easking for itself...
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
            //Check signal
            if (!WIFEXITED(last_status))
            {
                fprintf(stdout, "Latest process exited with signal: %d\n", WTERMSIG(last_status));
            }
            else
            {
                fprintf(stdout, "Latest process completed with status: %d\n", WEXITSTATUS(last_status));
            }
            fflush(stdout);
        }
        //If it's not a comment
        else if (program_name[0] != '#')
        {
            //Some pointers for arguments
            char *token;
            char *args[512];
            memset(args, '\0', 512);

            int arg_index = 0;
            args[arg_index++] = program_name;

            char pid_token[8];
            memset(pid_token, '\0', 8);

            //Parse all of our tokens
            while (token = (char *)strtok_r(NULL, cmd_delim, &tokenizer_buffer))
            {

                args[arg_index++] = token;
            }

            pid_t child = fork();

            if (child == -1)
            {
                //forking failed
                fprintf(stderr, "Failed to start file.");
                fflush(stderr);
            }
            else if (child > 0)
            {
                //In the parent
                if (foreground_only || args[arg_index - 1][0] != '&')
                {
                    waitpid(child, &last_status, 0);
                    if (!WIFEXITED(last_status))
                    {
                        fprintf(stdout, "Process exited with signal: %d\n", WTERMSIG(last_status));
                        fflush(stdout);
                    }
                }
                else
                {
                    //Add child pid to list of background processes
                    background_processes[background_process_index++] = child;

                    fprintf(stdout, "Backgrounded Process: %d\n", child);
                    fflush(stdout);
                }
            }
            else
            {
                //In the child

                //If they wanted to background it
                if (args[arg_index - 1][0] == '&')
                {
                    args[--arg_index] = NULL;
                }

                //Find out where to send input / output
                int input_descriptor = get_input_redirection(args, &arg_index);
                int output_descriptor = get_output_redirection(args, &arg_index);

                if (input_descriptor < 0 || output_descriptor < 0)
                {
                    exit(1);
                }

                //Redirect our input and output
                dup2(input_descriptor, 0);
                dup2(output_descriptor, 1);

                //Exec
                int result = execvp(program_name, args);

                //Close our files
                close(output_descriptor);
                close(input_descriptor);

                return result;
            }
        }
    }

    free(command);
    free(tokenizer_buffer);

    return 0;
}

int get_input_redirection(char **buffer, int *buffer_length)
{
    //Look for our token character in the list of args
    int found = 0;
    for (int i = 0; i < (*buffer_length) - 1; i++)
    {
        if (strcmp(buffer[i], "<") == 0)
        {
            found = i;
        }
    }

    int file_desc;
    //If it was found
    if (found != 0)
    {
        //Open the next argument
        file_desc = open(buffer[found + 1], O_RDONLY, 0667);

        //If that failed, print couldn't open and return failure
        if (file_desc < 0)
        {
            fprintf(stdout, "Couldn't open file for reading.\n");
            fflush(stdout);
            last_status = 1;
            return -1;
        }
    }
    //If it wasn't found
    else
    {
        //Open /dev/null
        file_desc = open("/dev/null", O_RDONLY);
        return file_desc;
    }

    //Remove our two arguments from the list
    for (int i = found; i < (*buffer_length) - 2; i++)
    {
        buffer[i] = buffer[i + 2];
    }

    //Set the last two places to null
    buffer[*buffer_length - 2] = NULL;
    buffer[*buffer_length - 1] = NULL;

    //Decrement length
    *buffer_length = *buffer_length - 2;
    return file_desc;
}
//Same exact thing as above function except that it returns stdout if the location isn't found
int get_output_redirection(char **buffer, int *buffer_length)
{

    int found = 0;
    for (int i = 0; i < (*buffer_length) - 1; i++)
    {
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
            fprintf(stdout, "Couldn't open file for writing.\n");
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

    //Print our prompt
    fprintf(stdout, "%c ", prompt_string);
    fflush(stdout);

    //Read input up to the length of the buffer
    fgets(buffer, 2048, stdin);


    char *pid_pos = strstr(buffer, "$$");

    int pid_index = pid_pos - buffer;
    int buffer_len = strlen(buffer);

    //Look for the $$ to replace pid
    if (pid_pos != NULL)
    {
        buffer[pid_index] = '\0';
        if (pid_index < buffer_len - 2)
        {
            //This part was much easier in rust
            char *after_pid_replace = malloc(sizeof(char) * 2048);
            memset(after_pid_replace, '\0', 2048);
            strcpy(after_pid_replace, buffer + pid_index + 2);
            pid_t a_pid = getpid();
            char *pid_str = malloc(sizeof(char) * 10);
            memset(pid_str, '\0', 10);
            sprintf(pid_str, "%d", a_pid);
            strcat(buffer, pid_str);
            strcat(buffer, after_pid_replace);
        }
    }

    char *newpos = strchr(buffer, '\n');
    if (newpos != NULL)
    {
        *newpos = '\0';
    }
}

//Signal handler:
//Sets itself up to handle the signals again.
void handle_sig(int sig)
{
    if (sig == 2)
    {
        fprintf(stdout, "\n");
        fflush(stdout);
        signal(SIGINT, handle_sig);
    }
    else if (sig == 20)
    {
        signal(SIGTSTP, handle_sig);
        if (!foreground_only)
        {
            fprintf(stdout, "Entering foreground only mode.\n");
            fflush(stdout);
            foreground_only = true;
        }
        else
        {
            fprintf(stdout, "Exiting foreground only mode.\n");
            fflush(stdout);
            foreground_only = false;
        }
    }
}
