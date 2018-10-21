use std::fs::{self, File};
use std::io;
use std::io::{BufRead, BufReader, Read, Write};
use std::path::Path;

#[derive(PartialEq)]
enum RoomType {
    StartRoom,
    MidRoom,
    EndRoom,
    InvalidRoom,
}

impl From<String> for RoomType {
    fn from(val: String) -> Self {
        match &*val {
            "START_ROOM" => RoomType::StartRoom,
            "MID_ROOM" => RoomType::MidRoom,
            "END_ROOM" => RoomType::EndRoom,
            _ => RoomType::InvalidRoom,
        }
    }
}

impl std::fmt::Display for RoomType {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(
            f,
            "{}",
            match self {
                RoomType::StartRoom => "START_ROOM",
                RoomType::MidRoom => "MID_ROOM",
                RoomType::EndRoom => "END_ROOM",
                RoomType::InvalidRoom => "INVALID_ROOM",
            }
        )
    }
}

struct Room {
    name: String,
    connections: Vec<String>,
    room_type: RoomType,
}

impl std::fmt::Display for Room {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        writeln!(f, "NAME: {}", self.name);
        for conn in &self.connections {
            writeln!(f, "CONNECTION: {}", conn);
        }
        writeln!(f, "ROOM TYPE: {}", self.room_type)
    }
}

impl Room {
    fn from_file<T: Read>(file: T) -> std::result::Result<Self, Box<std::error::Error>> {
        let mut room = Room {
            name: String::default(),
            connections: Vec::with_capacity(6),
            room_type: RoomType::MidRoom,
        };

        let bufread = BufReader::new(file);

        bufread
            .lines()
            .take_while(|line| line.is_ok())
            .for_each(|line| {
                let line: String = line.unwrap();
                match &line {
                    line if line.starts_with("ROOM NAME") => {
                        let line: String = line
                            .chars()
                            .skip_while(|a_char| a_char != &':')
                            .skip(2)
                            .collect();
                        room.name = line;
                    }
                    line if line.starts_with("CONNECTION") => {
                        let line: String = line
                            .chars()
                            .skip_while(|a_char| a_char != &':')
                            .skip(2)
                            .collect();
                        room.connections.push(line);
                    }
                    line if line.starts_with("ROOM TYPE") => {
                        let line: String = line
                            .chars()
                            .skip_while(|a_char| a_char != &':')
                            .skip(2)
                            .collect();
                        room.room_type = line.into();
                    }
                    _ => {}
                }
            });

        Ok(room)
    }
}

fn get_start_room(rooms: &Vec<Room>) -> Option<&Room> {
    for room in rooms {
        if room.room_type == RoomType::StartRoom {
            return Some(room);
        }
    }

    None
}

fn main() -> std::result::Result<(), Box<std::error::Error>> {
    let files = load_files(Path::new("./sasol.rooms.10251"))?;

    run_game(&files);

    Ok(())
}

fn run_game(rooms: &Vec<Room>) {
    let mut current_room = get_start_room(&rooms).unwrap();
    let mut steps = 0;
    let mut path: Vec<&Room> = Vec::new();

    while current_room.room_type != RoomType::EndRoom {
        current_room = prompt_and_move(&rooms, current_room);
        steps += 1;
        path.push(current_room);
    }

    println!("You made it to the end in {} steps!", steps);
    println!("You took the following path to get there: ");

    for room in path {
        println!("{}", room.name);
    }
}

fn prompt_and_move<'a>(rooms: &'a Vec<Room>, current_room: &'a Room) -> &'a Room {
    println!("You're in room: {}", current_room.name);

    print!("Connections: ");
    for connection in 0..current_room.connections.len() - 1 {
        print!("{}, ", current_room.connections[connection]);
    }

    println!(
        "{}",
        current_room.connections[current_room.connections.len() - 1]
    );
    print!("Enter command > ");
    io::stdout().flush().unwrap();

    let mut buffer = String::new();
    let stdin = io::stdin();
    let mut handle = stdin.lock();

    handle.read_line(&mut buffer).unwrap();
    buffer.truncate(buffer.len() - 1);

    println!("");
    for (index, room) in rooms.iter().enumerate() {
        if buffer == room.name {
            return &rooms[index];
        }
    }

    println!("That is not a connection.");

    current_room
}

fn load_files<T: AsRef<Path>>(
    folder_dir: T,
) -> std::result::Result<Vec<Room>, Box<std::error::Error>> {
    let mut rooms: Vec<Room> = Vec::with_capacity(7);
    for entry in fs::read_dir(folder_dir)? {
        let file_name = entry?;
        rooms.push(Room::from_file(File::open(file_name.path())?)?)
    }

    Ok(rooms)
}
