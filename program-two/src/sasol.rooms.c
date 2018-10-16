#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#define false 0
#define true 1

enum RoomType
{
    START_ROOM = 0,
    MID_ROOM = 1,
    END_ROOM = 2,
};

struct Room
{
    int name_index;
    struct Room **connections;
    enum RoomType type;
    int connection_count;
};

char *names[10] = {"dungeon", "castle", "shire", "poopdeck", "bedroom", "closet", "narnia", "whiterun", "skyrim", "vault"};

/* For safety, name and connections cannot be used after passed to this constructor */
struct Room *new_room(int name_index, enum RoomType type, int connection_count)
{
    struct Room *room = malloc(sizeof(struct Room) * 1);
    room->name_index = name_index,
    room->connections = malloc(sizeof(struct Room *) * 6);
    room->type = type;
    room->connection_count = connection_count;
    return room;
}

int name_picked(int new_index, int *picked_names, int total_picked)
{
    int i;
    for (i = 0; i < total_picked; i++)
    {
        if (picked_names[i] == new_index)
        {
            return false;
        }
    }

    picked_names[total_picked] = new_index;
    return true;
}

struct Room **generate_7_rooms()
{
    struct Room **rooms = malloc(sizeof(struct Room *) * 7);

    int picked_names[7];
    int i;
    for (i = 0; i < 7; i++)
    {
        picked_names[i] = -1;
    }

    for (i = 0; i < 7; i++)
    {
        int new_name_index = rand() % 10;
        while (name_picked(new_name_index, picked_names, i) == false)
        {
            new_name_index = rand() % 10;
        }

        enum RoomType type;
        if (i == 0)
        {
            type = START_ROOM;
        }
        else if (i == 6)
        {
            type = END_ROOM;
        }
        else
        {
            type = MID_ROOM;
        }

        rooms[i] = new_room(new_name_index, type, 0);
    }

    return rooms;
}

/* This is slow but frankly this program doesn't need to be that fast. */
char *concat_string(char *one, char *two)
{

    char *result = malloc(strlen(one) + strlen(two) + 1);

    strcpy(result, one);
    strcat(result, two);

    free(one);
    return result;
}

char *write_to_file(char *room_string, char *room_name)
{

    /*
    char snum[10];
     char *connection_string = sprintf(snum, "%d", pid); */

    char *filename = concat_string(room_name, "_room");
    FILE *file = fopen(filename, "w");
    int results = fputs(room_string, file);

    if (results == EOF)
    {
        /*  printf("Failure"); */
    }

    fclose(file);
    free(filename);
}

void write_room_to_file(struct Room room, char* folder_name)
{

    /*  Example setting for testing this function */
    char *file_format = malloc(sizeof(char) * 1024);
    file_format = concat_string(file_format, "ROOM NAME: ");
    file_format = concat_string(file_format, names[room.name_index]);

    file_format = concat_string(file_format, "\n");

    int i;
    for (i = 0; i < room.connection_count; i++)
    {
        char snum[2];
        sprintf(snum, "%d", i + 1);
        file_format = concat_string(file_format, "CONNECTION ");
        file_format = concat_string(file_format, snum);
        file_format = concat_string(file_format, ": ");
        file_format = concat_string(file_format, names[room.connections[i]->name_index]);
        file_format = concat_string(file_format, "\n");
    }

    file_format = concat_string(file_format, "ROOM TYPE: ");

    switch (room.type)
    {
    case START_ROOM:
        file_format = concat_string(file_format, "START_ROOM");
        break;

    case MID_ROOM:
        file_format = concat_string(file_format, "MID_ROOM");
        break;

    case END_ROOM:
        file_format = concat_string(file_format, "END_ROOM");
        break;
    }

    file_format = concat_string(file_format, "\n");

    /*  printf("%s\n", file_format); */

    char *folder_name_copy = malloc(sizeof(char) * 50);
    strcpy(folder_name_copy, folder_name);
    folder_name_copy = concat_string(folder_name_copy, "/");
    folder_name_copy = concat_string(folder_name_copy, names[room.name_index]);

    write_to_file(file_format, folder_name_copy);
    free(folder_name_copy);
    free(file_format);
}

/* The following code for generating a graph of rooms is outlined in 2.2 Program Outlining/* https/* oregonstate.instructure.com/courses/1725991/pages/2-dot-2-program-outlining-in-program-2/* Courtesy of Professor Brewster @ Oregon State University */
/* An array of rooms */

/* Gets a random index of the rooms to return */
struct Room *get_random_room(struct Room **rooms, int room_count)
{
    return rooms[rand() % room_count];
}

/* Adds each room to the end of the other room's connections */
void connect_rooms(struct Room *room_one, struct Room *room_two)
{
    /*  printf("Connecting rooms: %s - %s\n", names[room_one->name_index], names[room_two->name_index]); */

    room_one->connections[room_one->connection_count++] = room_two;

    room_two->connections[room_two->connection_count++] = room_one;
}

/* Tries to pick valid rooms to connect */
void add_connection(struct Room **rooms, int room_count)
{

    struct Room *room_one;
    struct Room *room_two;

    /*  printf("Selecting Room One\n"); */
    do
    {
        room_one = get_random_room(rooms, room_count);

    } while (can_add_connection(room_one) == false);

    /*  printf("Selecting Room Two\n"); */
    do
    {
        room_two = get_random_room(rooms, room_count);
    } while (can_add_connection(room_two) == false || is_same_room(room_one, room_two) == true || connection_exists(room_one, room_two) == true);

    connect_rooms(room_one, room_two);
}

void generate_graph(struct Room **rooms, int room_count)
{
    while (graph_is_full(rooms, room_count) == false)
    {
        add_connection(rooms, room_count);
    }
}

/* Checks if room's connection count is less than 6 */
int can_add_connection(struct Room *room)
{
    /*  printf("Checking Can Add\n"); */
    return room->connection_count < 6;
}
/* 0 for false, 1 for true */
int graph_is_full(struct Room **rooms, int room_count)
{

    int i;
    for (i = 0; i < room_count; i++)
    {
        if (rooms[i]->connection_count < 3)
        {
            return false;
        }
    }

    return true;
}

/* Returns true if the rooms have the same name and number of connections */
int is_same_room(struct Room *room_one, struct Room *room_two)
{
    /*  printf("Checking Is Same\n"); */
    return room_one->name_index == room_two->name_index;
}

int connection_exists(struct Room *room_one, struct Room *room_two)
{
    /*  printf("Checking Connection Exists\n"); */
    int i, j;
    for (i = 0; i < room_one->connection_count; i++)
    {
        /*  printf("Checking if %d is room %d\n", room_one->connections[i]->name_index, room_two->name_index); */
        if (is_same_room(room_one->connections[i], room_two) == true)
        {
            /*  printf("%d is %d\n", room_one->connections[i]->name_index, room_two->name_index); */
            return true;
        }
    }
    return false;
}

void make_folder(char* location) {
    struct stat st = {0};

    if (stat(location, &st) == -1)
    {
        mkdir(location, 0700);
    }
}

int main()
{
    srand(time(NULL));

    int pid = getpid();
    /* printf("Pid found: %d\n", pid); */

    struct Room **rooms = generate_7_rooms();

    generate_graph(rooms, 7);

    char *folder_name = malloc(sizeof(char) * 20);
    folder_name = concat_string(folder_name, "sasol.rooms.");

    char snum[10];
    sprintf(snum, "%d", pid);

    folder_name = concat_string(folder_name, snum);
    make_folder(folder_name);

    int i, j;
    for (i = 0; i < 7; i++)
    {
        write_room_to_file(*rooms[i], folder_name);
    }

    for (i = 0; i < 7; i++)
    {
        free(rooms[i]->connections);
        free(rooms[i]);
    }

    free(folder_name);
    free(rooms);
    return 0;
}
