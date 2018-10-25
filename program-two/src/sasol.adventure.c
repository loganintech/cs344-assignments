#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <pthread.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>

#define true 1
#define false 0
#define bool int

pthread_t thread_id;
pthread_mutex_t mutex_lock;
pthread_mutex_t second_mutex;

#pragma region loading_files
/*  Name array, same as */
char *names[10] = {"dungeon", "castle", "shire", "poopdeck", "bedroom", "closet", "narnia", "whiterun", "skyrim", "vault"};
/* Make us a struct with a name index, conn count, and spaces for chars of other details */
struct Room
{
    int name_index;
    int connection_count;

    char *room_type;
    char **connections;
};

char *extract_line_data(char *file_line, int line_length)
{
    /* Move up to the colon, then remove the colon and space. index is now right before the word we're extracting */
    int x, colon_index;
    for (x = 0; x < line_length; x++)
    {
        if (file_line[x] == ':')
        {
            colon_index = x + 2;
        }
    }

    /* Get a string for the offset length */
    char *return_string = malloc(sizeof(char) * (line_length - colon_index));
    memset(return_string, '\0', sizeof(char) * (line_length - colon_index));

    for (x = 0; x < (line_length - colon_index) - 1; x++)
    {
        return_string[x] = file_line[x + colon_index];
    }

    return return_string;
}

struct Room parse_file(char *file_path)
{

    FILE *file = fopen(file_path, "r");

    int line_length;

    if (file == NULL)
    {
        perror("Couldn't read a file.\n");
    }

    size_t len = 200;
    int line_count = 0;
    struct Room a_room;
    a_room.connection_count = 0;
    a_room.connections = malloc(sizeof(char *) * 6);

    char *buffer = malloc(sizeof(char) * 200);
    memset(buffer, '\0', sizeof(char) * 200);
    /* For every line */
    while ((line_length = getline(&buffer, &len, file)) != -1)
    {
        /* Get room name */
        if (line_count == 0)
        {
            char *room_name = extract_line_data(buffer, line_length);
            int x;
            for (x = 0; x < 10; x++)
            {
                if (strcmp(room_name, names[x]) == 0)
                {
                    a_room.name_index = x;
                }
            }
            free(room_name);
        }
        /* Get room connection */
        else if (buffer[0] == 'C')
        {
            a_room.connections[a_room.connection_count++] = extract_line_data(buffer, line_length);
        }
        /* Get room type */
        else
        {
            a_room.room_type = extract_line_data(buffer, line_length);
        }

        line_count++;
    }

    free(buffer);
    fclose(file);

    return a_room;
}

struct Room *read_files(char *path)
{
    if (chdir(path) != 0)
    {
        perror("Couldn't find a directory at that location.");
        exit(1);
    }

    char cwd[500];
    memset(cwd, '\0', sizeof(cwd));
    if (getcwd(cwd, sizeof(cwd)) == NULL)
    {
        perror("getcwd() error");
    }

    DIR *directory;
    struct dirent *dirent_struct;
    directory = opendir("./");

    struct Room *rooms = malloc(sizeof(struct Room) * 7);
    int room_index = 0;
    if (directory != NULL)
    {
        /* For every directory in the current directory */
        while (dirent_struct = readdir(directory))
        {
            /* Get a file's path */
            char *dir_path = malloc(sizeof(dirent_struct->d_name) + 250);
            memset(dir_path, '\0', sizeof(dir_path));
            sprintf(dir_path, "%s/%s", cwd, dirent_struct->d_name);
            if (dirent_struct->d_name[0] != '.' && strcmp(dirent_struct->d_name, "currentTime.txt") != 0)
            {
                /* Parse the room */
                rooms[room_index++] = parse_file(dir_path);
            }
            free(dir_path);
        }

        closedir(directory);
    }
    else
    {
        perror("Couldn't find a directory at that location.");
        exit(1);
    }

    return rooms;
}
/* Was useful during debugging. No real reason to delete, but no usage currently */
void print_room(struct Room room)
{

    printf("CURRENT ROOM: %s\n", names[room.name_index]);
    printf("POSSIBLE CONNECTIONS: ");

    int connection_index;
    for (connection_index = 0; connection_index < room.connection_count - 1; connection_index++)
    {
        printf("'%s'\n", room.connections[connection_index]);
    }

    printf("ROOM TYPE: %s\n", room.room_type);
}

#pragma endregion
struct Room prompt_and_move(struct Room *rooms, struct Room current_room)
{

    printf("CURRENT ROOM: %s\n", names[current_room.name_index]);
    printf("POSSIBLE CONNECTIONS: ");

    int connection_index;
    /* Print possible moves */
    for (connection_index = 0; connection_index < current_room.connection_count - 1; connection_index++)
    {
        printf("%s, ", current_room.connections[connection_index]);
    }

    printf("%s.\n", current_room.connections[connection_index]);
    printf("WHERE TO? >");

    char name_buffer[20];
    memset(name_buffer, '\0', sizeof(name_buffer));
    fgets(name_buffer, 20, stdin);

    name_buffer[strlen(name_buffer) - 1] = '\0';

    /* For every possible connection */
    for (connection_index = 0; connection_index < current_room.connection_count; connection_index++)
    {
        /* See if the choice is valid */
        if (strcmp(name_buffer, current_room.connections[connection_index]) == 0 || strcmp(name_buffer, "time") == 0)
        {
            int new_room_index;

            /* We know they picked a valid room now. Let's find its index and move */
            for (new_room_index = 0; new_room_index < 7; new_room_index++)
            {
                if (strcmp(name_buffer, names[rooms[new_room_index].name_index]) == 0)
                {
                    return rooms[new_room_index];
                }
                /* If they pick time */
                else if (strcmp(name_buffer, "time") == 0)
                {
                    /* printf("Unlocking mutex.\n"); fflush(stdout); */

                    int unlock_result = pthread_mutex_unlock(&mutex_lock);
                    if (unlock_result != 0)
                    {
                        printf("Unlocking failed.");
                        fflush(stdout);
                    }
                    /* Give the second thread time to grab the mutex */
                    sleep(2);
                    /* Then wait to grab the mutex back */
                    pthread_mutex_lock(&mutex_lock);

                    char *time_string = malloc(sizeof(char) * 1024);

                    /* Open the file for reading */
                    FILE *time_file = fopen("./currentTime.txt", "r");
                    memset(time_string, '\0', 1024);
                    size_t len = 1024;

                    /* Get the time line and print it out */
                    size_t line_length = getline(&time_string, &len, time_file);
                    time_string[line_length - 1] = '\0';

                    fclose(time_file);
                    printf("\n  %s\n", time_string);
                    free(time_string);
                    return current_room;
                }
            }
        }
    }

    printf("\nThat room is not a connection.\n");
    return current_room;
}

void run_game(struct Room *rooms)
{

    struct Room current_room;

    int i;
    int room_count = 7;
    /* Find our starting room */
    for (i = 0; i < room_count; i++)
    {
        if (rooms[i].room_type[0] == 'S')
        {
            current_room = rooms[i];
        }
    }

    /* If you take a path over 100 long idk how to help you */
    char *path_taken = malloc(sizeof(int) * 100);
    memset(path_taken, 0, sizeof(path_taken));

    int steps_taken = 0;
    while (current_room.room_type[0] != 'E')
    {
        current_room = prompt_and_move(rooms, current_room);
        printf("\n");
        path_taken[steps_taken] = current_room.name_index;

        /* Only add steps taken if it's a new path. This always increments at 0, and ignores repeated paths */
        if (steps_taken == 0 || (steps_taken > 0 && path_taken[steps_taken] != path_taken[steps_taken - 1]))
        {
            steps_taken++;
        }
    }

    printf("Congrats! You found the last room!\n");
    printf("You completed the game in %d steps.\nPath taken:\n", steps_taken);

    int path_step;
    /* Print all the paths */
    for (path_step = 0; path_step < steps_taken; path_step++)
    {
        printf("%s\n", names[path_taken[path_step]]);
    }

    free(path_taken);
}

#pragma endregion

void *write_time()
{
    while (true)
    {
        /* Lock our mutex */
        int lock_result = pthread_mutex_lock(&mutex_lock);
        if (lock_result != 0)
        {
            printf("Error locking in second thread.\n");
        }

        /* Try to lock the second (unlocked in main) if you can, break */
        size_t mutext_trylock = pthread_mutex_trylock(&second_mutex);
        if (mutext_trylock == 0)
        {
            break;
        }

        /* Open our file */
        FILE *time_file = fopen("./currentTime.txt", "w");

        const struct tm *right_now;
        time_t rawtime;
        time(&rawtime);

        /* Get our time */
        right_now = localtime(&rawtime);

        char time_string[200];
        memset(time_string, '\0', 200);
        char newline[1] = {'\n'};

        /* Format our time */
        size_t length = strftime(time_string, 200, "%I:%M%p, %A, %B %e, %Y", right_now);

        /* Write the time */
        fwrite(time_string, sizeof(char), length, time_file);
        fwrite(&newline, sizeof(char), 1, time_file);
        /* Close the file */
        fclose(time_file);
        pthread_mutex_unlock(&mutex_lock);
        /* Let the other thread lock the mutex */
        sleep(2);
    }

    return NULL;
}

char *get_directory()
{

    DIR *directory;
    struct dirent *dirent_struct;
    directory = opendir("./");

    /* Make space for a place to put in-dir filenames */
    char **filenames = malloc(sizeof(char *) * 200);
    memset(filenames, '\0', 200);
    int filename;
    for (filename = 0; filename < 200; filename++)
    {
        filenames[filename] = malloc(sizeof(char) * 100);
        memset(filenames[filename], '\0', 100);
    }
    filename = 0;

    int mtime = 0;
    int nameindex = 0;

    /* If we can actually open our own dir */
    if (directory != NULL)
    {
        /* Read through every file */
        while (dirent_struct = readdir(directory))
        {
            /* Ignore anything starting with . */
            if (dirent_struct->d_name[0] == '.')
                continue;

            /* See if the format is correct */
            if (strlen(dirent_struct->d_name) <= 12 || dirent_struct->d_name[5] != '.' || dirent_struct->d_name[11] != '.')
                continue;

            struct stat fileStat;
            /* Get the stat of this dir */
            if (stat(dirent_struct->d_name, &fileStat) < 0)
            {
                printf("Stat didn't work for file %s.", dirent_struct->d_name);
                continue;
            }

            /* If the currently selected dir is newer, set that as the one to save */
            if (mtime < fileStat.st_mtime)
            {
                mtime = fileStat.st_mtime;
                nameindex = filename;
            }

            strcpy(filenames[filename++], dirent_struct->d_name);
        }

        closedir(directory);
    }

    /* Make space for our filename. */
    char *real_filename = malloc(sizeof(char) * 256);

    /* Copy `./` to the filename */
    strcpy(real_filename, "./");

    /* Copy the actual folder name to everything after `./` */
    strcpy(real_filename + 2, filenames[nameindex]);

    int string_index;
    for (string_index = 0; string_index < 200; string_index++)
    {
        free(filenames[string_index]);
    }

    free(filenames);

    return real_filename;
}

int main(char *argv)
{
    /* Get our directory location */
    char *directory_location = get_directory();
    if (strlen(directory_location) <= 0)
    {
        perror("Cannot find valid directory in current path.\n");
        free(directory_location);
        return 3;
    }

    /* Read and parse the files from the directory */
    struct Room *rooms = read_files(directory_location);
    free(directory_location);
    if (pthread_mutex_init(&mutex_lock, NULL) != 0)
    {
        printf("Mutex creation failed.\n");
        return 1;
    }
    /* Lock our mutex */
    pthread_mutex_lock(&mutex_lock);

    /* Make a second mutex */
    if (pthread_mutex_init(&second_mutex, NULL) != 0)
    {
        printf("Mutex creation failed.\n");
        return 2;
    }
    /* Lock that one too */
    pthread_mutex_lock(&second_mutex);

    /* Create our other thread */
    int pthread_err = pthread_create(&thread_id, NULL, write_time, NULL);
    if (pthread_err != 0)
    {
        printf("Can't create thread: %s\n", strerror(pthread_err));
    }

    run_game(rooms);
    /* Unlcok the first mutex */
    pthread_mutex_unlock(&mutex_lock);
    /* Unlock the second. This causes the second thread to exit */
    pthread_mutex_unlock(&second_mutex);
    /* And, wait for it to exit. */
    pthread_join(thread_id, NULL);

    /* Delete everything */
    int room_index, connection_index;
    for (room_index = 0; room_index < 7; room_index++)
    {
        for (connection_index = 0; connection_index < rooms[room_index].connection_count; connection_index++)
        {
            free(rooms[room_index].connections[connection_index]);
        }
        free(rooms[room_index].connections);
        free(rooms[room_index].room_type);
    }

    free(rooms);

    return 0;
}
