#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>

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

    /* printf("Loading file: %s\n", file_path); */

    /*  char* dir_path = malloc(sizeof(file_path + 3)); */
    /*  sprintf(dir_path, "%s", file_path); */

    FILE *file = fopen(file_path, "r");

    int line_length;

    /* printf("Checking if file is null\n"); */
    if (file == NULL)
    {
        perror("Couldn't read a file.\n");
    }

    /* printf("Null check on file passed\n"); */

    size_t len;
    int line_count = 0;
    struct Room a_room;
    a_room.connection_count = 0;
    a_room.connections = malloc(sizeof(char *) * 6);

    char *buffer = malloc(sizeof(char) * 200);
    memset(buffer, '\0', sizeof(buffer));
    while ((line_length = getline(&buffer, &len, file)) != -1)
    {
        /* printf("Line Len: %d\n", line_length); */
        char *room_name;
        /* If we are looking for room name */
        if (line_count == 0)
        {
            room_name = extract_room_name(buffer, line_length);
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
        /* If we are looking for a connection name */
        else if (buffer[0] == 'C')
        {
            a_room.connections[a_room.connection_count++] = extract_room_name(buffer, line_length);
        }
        /* We're looking at room type now */
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

        if (steps_taken == 0 || (steps_taken > 0 && path_taken[steps_taken] != path_taken[steps_taken - 1])) {
            steps_taken++;
        }
    }

    printf("Congrats! You found the last room!\n");
    printf("You completed the game in %d steps.\nPath taken:\n", steps_taken);

    int path_step;
    for (path_step = 0; path_step < steps_taken; path_step++) {
        printf("%s\n", names[path_taken[path_step]]);
    }

    free(path_taken);
}

int main(char *argv)
{
    struct Room *rooms = read_files("./sasol.rooms.19818");
    run_game(rooms);

    free(rooms);

    return 0;
}
