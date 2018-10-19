extern crate rand;

use rand::prelude::*;

use std::ffi::OsStr;
use std::fmt;
use std::path::Path;
use std::process::id;
use std::fs::{create_dir, write};

#[derive(Clone)]
enum RoomType {
    StartRoom,
    MidRoom,
    EndRoom,
}

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

const ROOM_NAMES: [&str; 10] = [
    "dungeon", "castle", "shire", "poopdeck", "bedroom", "closet", "narnia", "whiterun", "skyrim",
    "vault",
];

#[derive(Clone)]
struct Room {
    name_index: u8,
    connections: Vec<u8>,
    room_type: RoomType,
}

impl Room {
    fn new(name_index: u8, room_type: RoomType) -> Self {
        Room {
            name_index,
            room_type,
            connections: Vec::with_capacity(6),
        }
    }

    fn get_multiple_rooms(capacity: usize) -> Vec<Self> {
        let mut num_vec: Vec<u8> = Vec::with_capacity(capacity);
        let mut rng = thread_rng();

        while num_vec.len() < 7 {
            let new_num = (rng.next_u32() % 10) as u8;
            if num_vec.contains(&new_num) {
                continue;
            }

            num_vec.push(new_num);
        }

        num_vec
            .into_iter()
            .enumerate()
            .map(|(index, name_index)| {
                Room::new(
                    name_index as u8,
                    match index {
                        0 => RoomType::StartRoom,
                        x if x > 0 && x < capacity - 1 => RoomType::MidRoom,
                        _ => RoomType::EndRoom,
                    },
                )
            }).collect()
    }

    fn connect_rooms(&mut self, second_room_index: u8) -> Option<()> {
        if !Room::can_add_connection(self) {
            return None;
        }

        self.connections.push(second_room_index);

        Some(())
    }

    fn can_add_connection(room: &Room) -> bool {
        return room.connections.len() < 6;
    }
}

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

impl PartialEq for Room {
    fn eq(&self, other: &Room) -> bool {
        self.name_index == other.name_index
    }
}

fn write_room_to_file<T: AsRef<Path> + AsRef<OsStr>>(
    room: &Room,
    folder_name: T,
) -> std::io::Result<()> {
    let path = Path::new(&folder_name);
    write(
        path.join(format!("{}_room", ROOM_NAMES[room.name_index as usize])),
        format!("{}", room),
    )
}

fn get_random_room(rooms: &mut Vec<Room>) -> usize {
    let length = rooms.len() as usize;
    thread_rng().next_u32() as usize % length
}

fn graph_is_full(rooms: &Vec<Room>) -> bool {
    rooms.iter().all(|room| room.connections.len() >= 3)
}

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

        while !Room::can_add_connection(&rooms[second_room]) || first_room == second_room || connection_exists(&rooms[first_room], &rooms[second_room]) {
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
        .for_each(|room| write_room_to_file(room, folder_name.clone()).unwrap());

    Ok(())
}
