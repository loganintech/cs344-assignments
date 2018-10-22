#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <time.h>

#define true 1
#define false 0
#define bool int

pthread_t thread_id;
pthread_mutex_t mutex_lock;

#pragma region create_rooms

char *names[10] = {"dungeon", "castle", "shire", "poopdeck", "bedroom", "closet", "narnia", "whiterun", "skyrim", "vault"};
struct Room
{
    int name_index;
    int connection_count;

    char *room_type;
    char **connections;
};

char *extract_room_name(char *file_line, int line_length)
{
    int x, colon_index;
    for (x = 0; x < line_length; x++)
    {
        if (file_line[x] == ':')
        {
            colon_index = x + 2;
        }
    }

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
    while ((line_length = getline(&buffer, &len, file)) != -1)
    {
        if (line_count == 0)
        {
            char *room_name = extract_room_name(buffer, line_length);
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
        else if (buffer[0] == 'C')
        {
            a_room.connections[a_room.connection_count++] = extract_room_name(buffer, line_length);
        }
        else
        {
            a_room.room_type = extract_room_name(buffer, line_length);
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
        while (dirent_struct = readdir(directory))
        {
            char *dir_path = malloc(sizeof(dirent_struct->d_name) + 250);
            memset(dir_path, '\0', sizeof(dir_path));
            sprintf(dir_path, "%s/%s", cwd, dirent_struct->d_name);
            if (dirent_struct->d_name[0] != '.')
            {
                /* printf("Reading File: %s\n", dirent_struct->d_name); */
                /* printf("About to parse file: %s\n", dirent_struct->d_name); */
                rooms[room_index++] = parse_file(dir_path);
                /* print_room(rooms[room_index - 1]); */
                /* printf("Just parsed file: %s\n", dirent_struct->d_name); */
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

    int new_room_index;
    for (new_room_index = 0; new_room_index < 7; new_room_index++)
    {
        if (strcmp(name_buffer, names[rooms[new_room_index].name_index]) == 0)
        {
            return rooms[new_room_index];
        } else if (strcmp(name_buffer, "time") == 0) {
            printf("Unlocking mutex.");

            pthread_mutex_unlock(&mutex_lock);
            sleep(1);
            pthread_mutex_lock(&mutex_lock);

            FILE *time_file = fopen("./currentTime.txt", "r");
            char *time_string = malloc(sizeof(char) * 100);

            size_t line_length = getline(&time_string, sizeof(char) * 100, time_file);

            printf("\n%s\n", time_string);
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

        if (steps_taken == 0 || (steps_taken > 0 && path_taken[steps_taken] != path_taken[steps_taken - 1]))
        {
            steps_taken++;
        }
    }

    printf("Congrats! You found the last room!\n");
    printf("You completed the game in %d steps.\nPath taken:\n", steps_taken);

    int path_step;
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
        pthread_mutex_lock(&mutex_lock);

        FILE *time_file = fopen("./currentTime.txt", "w");

        const struct tm *right_now;
        time_t rawtime;
        time(&rawtime);
        right_now = localtime(&rawtime);

        /* 1:03pm, Tuesday, September 13, 2016 */

        char *time_string = malloc(sizeof(char) * 100);
        size_t length = strftime(time_string, sizeof(time_string), "%I:%M%p, %A, %m %e, %Y", right_now);
        fwrite(time_string, sizeof(char), sizeof(time_string), time_file);
        fclose(time_file);
        pthread_mutex_unlock(&mutex_lock);
    }

    return NULL;
}

int main(char *argv)
{
    struct Room *rooms = read_files("./sasol.rooms.19818");
    if (pthread_mutex_init(&mutex_lock, NULL) != 0)
    {
        printf("\nMutex creation failed.\n");
        return 1;
    }
    printf("Created mutex");

    int pthread_err = pthread_create(&thread_id, NULL, write_time, NULL);
    if (pthread_err != 0) {
        printf("Can't create thread: %s", strerror(pthread_err));
    }

    pthread_mutex_lock(&mutex_lock);
    run_game(rooms);
    pthread_mutex_unlock(&mutex_lock);

    pthread_kill(&thread_id, SIGKILL);

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

