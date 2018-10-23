extern crate rand;

use rand::prelude::*;

use std::ffi::OsStr;
use std::fmt;
use std::path::Path;
use std::process::id;
use std::fs::{create_dir, write};

//The derive line has the rust compiler implement `Clone` and `Copy` for us
//Clone says this copy can be duplicated. For something to be `Clone`, all of its fields must be `Clone`
//We derive `Clone` here so that we can do it for `Room` later
//Copy says that duplicating this type is as simple as making a byte-for-byte copy of it from the stack
//This is an enum, which is represented internally as a number, so we can derive `Copy`
#[derive(Clone, Copy)]
enum RoomType {
    StartRoom,
    MidRoom,
    EndRoom,
}

//I added this to make it easier to format later on in the program
impl fmt::Display for RoomType {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(
            f,
            "{}",
            match self {
                RoomType::StartRoom => "START_ROOM",
                RoomType::MidRoom => "MID_ROOM",
                RoomType::EndRoom => "END_ROOM",
            }
        )
    }
}

//A fixed-length array of size `10` that holds all of the room names.
const ROOM_NAMES: [&str; 10] = [
    "dungeon", "castle", "shire", "poopdeck", "bedroom", "closet", "narnia", "whiterun", "skyrim",
    "vault",
];

//We derive clone here so that we can make clones of it for the grid
//We keep a list of u8s as the connection variables.
//This method is similar to what was described in class
#[derive(Clone)]
struct Room {
    name_index: u8,
    connections: Vec<u8>,
    room_type: RoomType,
}

impl Room {
    //Create a new room with a name index and no connections (but has space for connections)
    fn new(name_index: u8, room_type: RoomType) -> Self {
        Room {
            name_index,
            room_type,
            connections: Vec::with_capacity(6),
        }
    }

    //This is not really a rust-y function. I just did this for simplicity's sake.
    fn get_multiple_rooms(capacity: usize) -> Vec<Self> {
        //Create a list of room names with num_vec.
        let mut num_vec: Vec<u8> = Vec::with_capacity(capacity);
        //We are using the `random` crate. This function thread_rng creates well-distributed numbers
        let mut rng = thread_rng();

        while num_vec.len() < capacity || num_vec.len() < 10 {
            let new_num = (rng.next_u32() % 10) as u8;
            if num_vec.contains(&new_num) {
                continue;
            }

            num_vec.push(new_num);
        }

        //For every index in our list of numbers
        num_vec
            .into_iter()
            //We also want to keep an index of where we are as we go, so let's .enumerate()
            .enumerate()
            //`map` is intended to translate an interator of one type into an iterator of another
            .map(|(index, name_index)| {
                //Create a new room with the name_index from the ones we generated earlier
                Room::new(
                    name_index as u8,
                    //Then match our index. If our index is 0, it's start room. If our index is greater than 0 and less than the total capacity minus the last one, it's a midroom. Otherwise (if it's the last one) it's an endroom
                    match index {
                        0 => RoomType::StartRoom,
                        x if x > 0 && x < capacity - 1 => RoomType::MidRoom,
                        _ => RoomType::EndRoom,
                    },
                )
            })
            //Collect all of those into a Vec<Self> which is Vec<Room>
            .collect()
    }

    fn connect_rooms(&mut self, second_room_index: u8) -> Option<()> {
        //If we can't add connection, return None
        if !Room::can_add_connection(self) {
            return None;
        }

        //Add the index to our connections
        self.connections.push(second_room_index);

        //Return success
        Some(())
    }

    fn can_add_connection(room: &Room) -> bool {
        return room.connections.len() < 6;
    }
}

//Print the name. This was useful in debugging
impl fmt::Display for Room {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        let mut room_string: String =
            format!("ROOM NAME: {}\n", ROOM_NAMES[self.name_index as usize]);

        for (index, connection) in self.connections.iter().enumerate() {
            room_string.push_str(&format!(
                "CONNECTION {}: {}\n",
                index, ROOM_NAMES[*connection as usize]
            ));
        }

        room_string.push_str(&format!("ROOM TYPE: {}\n", self.room_type));

        write!(f, "{}", room_string)
    }
}

//Implement partial equality for room, comparing the name indicies
impl PartialEq for Room {
    fn eq(&self, other: &Room) -> bool {
        self.name_index == other.name_index
    }
}

//This takes a generic type T that is compatible as a path and an operating system string. AKA, you can't write a file the OS can't suport
fn write_room_to_file<T: AsRef<Path> + AsRef<OsStr>>(
    room: &Room,
    folder_name: T,
) -> std::io::Result<()> {
    let path = Path::new(&folder_name);
    //Write our data to our file by joining the path and the file name, then use format!() to run fmt Display trait we implemented earlier to translate a room into a string
    write(
        path.join(format!("{}_room", ROOM_NAMES[room.name_index as usize])),
        format!("{}", room),
    )
}

fn get_random_room(rooms: &mut Vec<Room>) -> usize {
    let length = rooms.len() as usize;
    thread_rng().next_u32() as usize % length
}

//If all values (in this case a room) in the iterator pass the test, return true. Otherwise return false.
fn graph_is_full(rooms: &Vec<Room>) -> bool {
    rooms.iter().all(|room| room.connections.len() >= 3)
}

//This function is basically the exact same as the C version.
fn generate_graph(rooms: &mut Vec<Room>) {
    let mut first_room;
    let mut second_room = get_random_room(rooms);

    while !graph_is_full(rooms) {
        loop {
            first_room = get_random_room(rooms);

            if Room::can_add_connection(&rooms[first_room]) {
                break;
            }
        }

        while !Room::can_add_connection(&rooms[second_room])
            || first_room == second_room
            || connection_exists(&rooms[first_room], &rooms[second_room])
        {
            second_room = get_random_room(rooms);
        }

        let second_room_index = rooms[second_room].name_index;
        let first_room_index = rooms[first_room].name_index;

        rooms[first_room].connect_rooms(second_room_index);
        rooms[second_room].connect_rooms(first_room_index);
    }
}

fn connection_exists(first_room: &Room, second_room: &Room) -> bool {
    for conn in &first_room.connections {
        if conn == &second_room.name_index {
            return true;
        }
    }

    return false;
}

fn main() -> Result<(), Box<std::error::Error>> {
    let mut rooms: Vec<Room> = Room::get_multiple_rooms(7);

    generate_graph(&mut rooms);

    let pid = id();
    let folder_name = format!("sasol.rooms.{}", pid);

    create_dir(folder_name.clone())?;

    rooms
        .iter()
        //For every room, write it to file
        .for_each(|room| write_room_to_file(room, folder_name.clone()).unwrap());

    Ok(())
}
