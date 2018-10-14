#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

enum RoomType
{
    START_ROOM = 0,
    MID_ROOM = 1,
    END_ROOM = 2,
};

struct Room
{
    char *name;
    struct Room *connections;
    enum RoomType type;
    int connection_count;
};

char *names[10] = {"dungeon", "castle", "shire", "poopdeck", "bedroom", "closet", "narnia", "whiterun", "skyrim", "vault"};

//For safety, name and connections cannot be used after passed to this constructor
struct Room new_room(char *name, struct Room *connections, enum RoomType type, int connection_count)
{
    struct Room room; //= malloc(sizeof(struct Room) * 1);
    room.name = name,
    room.connections = connections;
    room.type = type;
    room.connection_count = connection_count;
    return room;
}

struct Room *generate_7_rooms()
{
    struct Room *rooms = malloc(sizeof(struct Room) * 7);

    for (int i = 0; i < 7; i++)
    {

        rooms[i] = new_room(names[i], NULL, START_ROOM, 0);
    }

    return rooms;
}

//This is slow but frankly this program doesn't need to be that fast.
char *concat_string(char *one, char *two)
{

    char *result = malloc(strlen(one) + strlen(two) + 1);

    strcpy(result, one);
    strcat(result, two);

    free(one);
    return result;
}

char *write_to_file(char *room_string, char* room_name) {
    // int pid = getpid();
    // char snum[10];
    // char *connection_string = sprintf(snum, "%d", pid);


    FILE *file = fopen(concat_string(room_name, "_room"), "w");
    int results = fputs(room_string, file);

    if (results == EOF) {
        printf("Fuck");
    }

    fclose(file);

}

void print_room(struct Room room)
{

    // Example setting for testing this function
    int connection_count = 3;

    char *file_format = concat_string("ROOM NAME: ", room.name);

    file_format = concat_string(file_format, "\n");

    for (int i = 0; i < connection_count && i < 10; i++)
    {
        char snum[2];
        char *connection_string = (char*) sprintf(snum, "%d", i + 1);
        file_format = concat_string(file_format, "CONNECTION ");
        file_format = concat_string(file_format, snum);
        file_format = concat_string(file_format, ": ");
        file_format = concat_string(file_format, room.name);
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

    printf("%s\n", file_format);

    char room_name[10];
    write_to_file(file_format, room.name);

    free(file_format);
    // printf("ROOM NAME: %s\nCONNECTION: %d\nROOM TYPE: %s", room.name, );
}

int main()
{
    // printf("Pid found: %d\n", pid);

    struct Room *rooms = generate_7_rooms();
    for (int i = 0; i < 7; i++)
    {
        print_room(rooms[i]);
    }

    free(rooms);
    return 0;
}
