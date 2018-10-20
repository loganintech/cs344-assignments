use std::fs::{self, DirEntry, File};
use std::io::{Read, BufReader, BufRead};
use std::path::Path;
use std::io;

enum RoomType {
    START_ROOM,
    MID_ROOM,
    END_ROOM,
    INVALID_ROOM,
}

impl From<String> for RoomType {
    fn from(val: String) -> Self {
        match &*val {
            "START_ROOM" => RoomType::START_ROOM,
            "MID_ROOM" => RoomType::MID_ROOM,
            "END_ROOM" => RoomType::END_ROOM,
            _ => RoomType::INVALID_ROOM,
        }
    }
}

struct Room {

    name: String,
    connections: Vec<String>,
    room_type: RoomType,

}


impl Room {
    fn from_file<T: Read>(file: T) -> std::result::Result<Self, Box<std::error::Error>> {

        let mut room = Room {
            name: String::default(),
            connections: Vec::with_capacity(6),
            room_type: RoomType::MID_ROOM,
        };

        let bufread = BufReader::new(file);

        bufread.lines()
        .take_while(|&line| line.is_ok())
        .map(|line| {
            let line: String = line.unwrap();
            let line: String = line.chars().skip_while(|a_char| a_char != &':').skip(1).collect();
            line
        })
        .enumerate()
        .map(|(index, val)| {
            match (index, &val) {
                (0, _) => room.name = val,
                (x, val) if !val.is_uppercase() => room.connections.push(val.clone()),
                _ => room.room_type = val.into(),
            }
        });


        Ok(room)

    }
}

fn main() -> std::result::Result<(), Box<std::error::Error>> {
    println!("Hello, world!");

    let files = load_files(Path::new("./sasol.rooms.10251"))?;

    Ok(())
}

fn load_files<T: AsRef<Path>>(folder_dir: T) -> std::result::Result<Vec<Room>, Box<std::error::Error>> {

    let rooms: Vec<Room> = Vec::with_capacity(7);
    for entry in fs::read_dir(folder_dir)? {
        let file_name = entry?;
        rooms.push(Room::from_file(File::open(file_name.path())?)?)
    }

    Ok(rooms)
}
