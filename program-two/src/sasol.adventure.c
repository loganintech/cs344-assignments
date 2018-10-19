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
    char *room_type;
    int connection_count;
    char **connections;
};

char *extract_room_name(char *file_line, int line_length)
{
    int x, colon_index;
    for (x = 0; x < line_length; x++)
    {
        if (file_line[x] == ':')
        {
            colon_index = x;
        }
    }

    /* Normally you'd need +1 for the null terminator, but the colon is before a space, so the space can be the + 1 */
    char *return_string = malloc(sizeof(char) * (line_length - colon_index));

    for (x = 0; x < line_length - colon_index; x++)
    {
        return_string[x] = file_line[x + colon_index + 1];
    }

    return return_string;
}

struct Room parse_file(char *file_path)
{

    /* printf("Loading file: %s\n", file_path); */

    /*  char* dir_path = malloc(sizeof(file_path + 3)); */
    /*  sprintf(dir_path, "%s", file_path); */

    FILE *file = fopen(file_path, "r");
    char *buffer = malloc(sizeof(char) * 100);

    int buf;
    for (buf = 0; buf < 100; buf++)
    {
        buffer[buf] = '\0';
    }

    size_t len;
    int line_length;

    /* printf("Checking if file is null\n"); */
    if (file == NULL)
    {
        perror("Couldn't read a file.\n");
    }

    /* printf("Null check on file passed\n"); */

    int line_count = 0;
    struct Room a_room;
    a_room.connection_count = 0;
    a_room.connections = malloc(sizeof(char *) * 6);
    while ((line_length = getline(&buffer, &len, file)) != -1)
    {
        /* printf("Line Len: %d\n", line_length); */
        char *room_name;
        /* If we are looking for room name */
        if (line_count == 0)
        {
            room_name = extract_room_name(buffer, line_length);

            int x;
            for (x = 0; x < strlen(room_name); x++)
            {
                if (room_name == names[x])
                {
                    a_room.name_index = x;
                }
            }
        }
        /* If we are looking for a connection name */
        else if (buffer[0] == 'C')
        {
            int x;
            for (x = 0; x < strlen(room_name); x++)
            {
                a_room.connections[a_room.connection_count++] = extract_room_name(buffer, line_length);
            }
        }
        /* We're looking at room type now */
        else
        {
            a_room.room_type = extract_room_name(buffer, line_length);
        }

        line_count++;
    }

    return a_room;
}

struct Room* read_files(char *path)
{
    if (chdir(path) != 0)
    {
        perror("Couldn't find a directory at that location.");
        exit(1);
    }

    char cwd[500];
    if (getcwd(cwd, sizeof(cwd)) == NULL)
    {
        perror("getcwd() error");
    }

    DIR *directory;
    struct dirent *dirent_struct;
    directory = opendir("./");

    struct Room* rooms = malloc(sizeof(struct Room) * 7);
    int room_index = 0;
    if (directory != NULL)
    {
        while (dirent_struct = readdir(directory))
        {
            printf("%s\n", dirent_struct->d_name);
            char *dir_path = malloc(sizeof(dirent_struct->d_name) + 250);
            sprintf(dir_path, "%s/%s", cwd, dirent_struct->d_name);
            if (dirent_struct->d_name[0] != '.')
            {
                /* printf("About to parse file: %s\n", dirent_struct->d_name); */
                rooms[room_index++] = parse_file(dir_path);
                /* printf("Just parsed file: %s\n", dirent_struct->d_name); */
            }

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

struct Room prompt_and_move(struct Room* rooms, struct Room current_room) {

    printf("CURRENT ROOM: %s\n", names[current_room.name_index]);
    printf("POSSIBLE CONNECTIONS: ");

    int connection_index;
    for(connection_index = 0; connection_index < current_room.connection_count - 1; connection_index++) {

        printf("%s, ", current_room.connections[connection_index]);

    }

    printf("%s.\n", current_room.connections[connection_index]);
    printf("WHERE TO? >");

    char name_buffer[20];
    fgets(name_buffer, 20, stdin);


    int new_room_index;
    for (new_room_index = 0; new_room_index < 7; new_room_index++) {
       if (strcmp(name_buffer, names[current_room.name_index]) == 0) {
           return rooms[new_room_index];
       }
    }

    printf("That room is not a connection.");
    return current_room;
}

void run_game(struct Room* rooms) {

    struct Room current_room;

    int i;
    int room_count = 7;
    for (i = 0; i < room_count; i++)
    {
        printf("Rooms!");
        if (rooms[i].room_type[0] == 'S')
        {
            current_room = rooms[i];
        }
    }

    /* If you take a path over 100 long idk how to help you */
    char** path_taken = malloc(sizeof(char*) * 100);

    int steps_taken;
    while (current_room.room_type[0] != 'E') {
        current_room = prompt_and_move(rooms, current_room);
        path_taken[steps_taken] = names[current_room.name_index];
        steps_taken++;
    }

    printf("Congrats! You found the last room!");

}

int main(char *argv)
{

    struct Room* rooms = read_files("./sasol.rooms.19818");
    run_game(rooms);

    return 0;
}
