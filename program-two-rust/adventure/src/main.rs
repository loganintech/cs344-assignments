use std::fs::{self, File};
use std::io;
use std::io::{BufRead, BufReader, Read, Write};
use std::path::Path;
use std::thread;
use std::time::{SystemTime, UNIX_EPOCH};

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

impl PartialEq for Room {
    fn eq(&self, rhs: &Room) -> bool {
        self.name == rhs.name
    }
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
                        let line: String = extract_value(line);
                        room.name = line;
                    }
                    line if line.starts_with("CONNECTION") => {
                        let line: String = extract_value(line);
                        room.connections.push(line);
                    }
                    line if line.starts_with("ROOM TYPE") => {
                        let line: String = extract_value(line);
                        room.room_type = line.into();
                    }
                    line => println!("Erronious line in file: {}", line),

                }
            });

        Ok(room)
    }
}

fn extract_value(line: &String) -> String {
    line.chars()
        .skip_while(|a_char| a_char != &':')
        .skip(2)
        .collect()
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

fn write_time() -> io::Result<String> {
    let start = SystemTime::now();
    let since_epoch = start.duration_since(UNIX_EPOCH).expect("Weird time issue.");

    let mut time_file = File::create("./currentTime.txt")?;
    let time_string = format!("{}", since_epoch.as_secs());
    time_file.write_all(time_string.as_bytes())?;

    Ok(time_string)
}

fn run_game(rooms: &Vec<Room>) {
    let mut current_room = get_start_room(&rooms).unwrap();
    let mut steps = 0;
    let mut path: Vec<&Room> = Vec::new();

    while current_room.room_type != RoomType::EndRoom {
        match prompt_and_move(&rooms, current_room) {
            Some(x) => {
                if x != current_room {
                    current_room = x;
                    steps += 1;
                    path.push(current_room);
                }
            }
            None => match thread::spawn(write_time).join().unwrap() {
                Ok(time) => println!("Time written to file: {}", time),
                Err(e) => eprintln!("Error occured writing time to file: {}", e),
            },
        }
    }

    println!("You made it to the end in {} steps!", steps);
    println!("You took the following path to get there: ");

    for room in path {
        println!("{}", room.name);
    }
}

fn prompt_and_move<'a>(rooms: &'a Vec<Room>, current_room: &'a Room) -> Option<&'a Room> {
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
    if buffer == "time" {
        return None;
    }

    for room_conn in current_room.connections.iter() {
        if &buffer == room_conn {
            for (index, room) in rooms.iter().enumerate() {
                if &room.name == room_conn {
                    return Some(&rooms[index]);
                }
            }
        }
    }

    println!("That is not a connection.");

    Some(current_room)
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
