use std::fs::{self, File};
use std::io;
use std::io::{BufRead, BufReader, Read, Write};
use std::path::{Path, PathBuf};
use std::thread;
use std::time::{SystemTime, UNIX_EPOCH};

//Make an enum and define PartialEq so it can be compared to itself
#[derive(PartialEq)]
enum RoomType {
    StartRoom,
    MidRoom,
    EndRoom,
    InvalidRoom,
}

//Implement the `From` trait which is Rust's idomatic way of defining translations
//Implementing `from` also provides `into`
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

//Implement display for our room. In this case it's the inverse of From<String>
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

//Create a room struct with a name, list of connections, and type
struct Room {
    name: String,
    connections: Vec<String>,
    room_type: RoomType,
}

//Implement partial equality for our rooms. For the use case, comparing if the names are the same is ok

impl PartialEq for Room {
    fn eq(&self, rhs: &Room) -> bool {
        self.name == rhs.name
    }
}

//Implement display for room. This is used so that we can pass a `Room` struct directly to println
//I used this mostly during my debugging but haven't really since then.
impl std::fmt::Display for Room {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        writeln!(f, "NAME: {}", self.name)?;
        for conn in &self.connections {
            writeln!(f, "CONNECTION: {}", conn)?;
        }
        writeln!(f, "ROOM TYPE: {}", self.room_type)
    }
}

//This is our implementation for the Room struct. We could add some more logic or functions in here but for our usage
impl Room {
    //Normally we'd use the `From` trait, but the `From` trait GUARENTEES SUCCESS
    //So we can't use that, because this may not succeed in parsing (which is why it returns a Result)
    fn from<T: Read>(read: T) -> Result<Self, Box<std::error::Error>> {
        //Create a new room with default string, most common room type, and a connections vec that is at least as much as our max
        //If our max was really large, we may just do a reasonable defaut so we don't use uncessesary memory
        let mut room = Room {
            name: String::default(),
            connections: Vec::with_capacity(6),
            room_type: RoomType::MidRoom,
        };

        //Create a buffer reader from our read
        let bufread = BufReader::new(read);

        bufread
            //For every line in our read
            .lines()
            //Grab things from the `lines` iterator until the line is not an `Ok` enum
            .take_while(|line| line.is_ok())
            //Parse the line
            .for_each(|line| {
                //Create a buffer for our item. We can unwrap safely here because we checked if line.is_ok earlier
                let line: String = line.unwrap();
                //Match a reference here instead of in the extract_value function
                //This is for cleaner code, but doesn't make a different from passing `match line` and then using extract_value(&line)
                //Each `if` in this is called a MATCH GUARD. A match guard protects a branch of `match` from matching without passing the guard
                //In this case, we want to check if each line is what we expect in the file
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
                    //This line has no match guard, meaning anything that doesn't pass the other options dumps itself here
                    //Whatever line this is isn't something we expected
                    line => println!("Erronious line in file: {}", line),
                }
            });

        Ok(room)
    }
}

//This skips characters until the character is a colon, then it skips two more to get to the start of our property and collects the rest of the line into `String`
fn extract_value(line: &String) -> String {
    line.chars()
        .skip_while(|a_char| a_char != &':')
        .skip(2)
        .collect()
}

//Loop through a reference to all the rooms, returning a reference to the room that is a StartRoom or None if there isn't any
fn get_start_room(rooms: &Vec<Room>) -> Option<&Room> {
    for room in rooms {
        if room.room_type == RoomType::StartRoom {
            return Some(room);
        }
    }

    None
}

fn main() -> Result<(), Box<std::error::Error>> {
    let files = load_files(get_latest_directory().unwrap())?;

    run_game(&files);

    Ok(())
}

//Gets the current time, gets epoch, creates the file (deleting old ones) and writes the epoch to the file
fn write_time() -> io::Result<String> {
    let start = SystemTime::now();
    let since_epoch = start.duration_since(UNIX_EPOCH).expect("Weird time issue.");

    let mut time_file = File::create("./currentTime.txt")?;
    let time_string = format!("{}", since_epoch.as_secs());
    time_file.write_all(time_string.as_bytes())?;

    Ok(time_string)
}

//Takes a vector of rooms to use
fn run_game(rooms: &Vec<Room>) {
    //Get a reference to our start room;
    let mut current_room = get_start_room(&rooms).unwrap();
    //Setup an int for our steps
    let mut steps = 0;
    //Setup a list of room references. This is dropped before `rooms`, which means we can do this. If we were to move rooms into a function or variable in this function we'd get errors
    let mut path: Vec<&Room> = Vec::new();

    //While we're not at the end
    while current_room.room_type != RoomType::EndRoom {
        //Run the prompt and move function. If it returns Some(x), we move to that room and increase steps (as long as it's not the same as the previous one)
        match prompt_for_game_action(&rooms, current_room) {
            Some(x) => {
                if x != current_room {
                    current_room = x;
                    steps += 1;
                    path.push(current_room);
                }
            }
            //If it returns none that means we entered the `time` command
            //Start a new thread that runs the `write_time` function, joins it, and unwraps the result
            //If the second thread fails it would fail. We could use a nested match here to ensure we handle every possibility
            //In rust, the function `.unwrap` is a conscious decision to say "If there was an error here, the program should crash"
            //In this case it's me being a bit lazy
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

//Ok, this wasn't in the last program. What the heck is this <'a> you ask
//This is called a LIFETIME. Lifetimes are something that exists in every language, but is a specific construct in rust.
//This tells the rust compiler that the reference to what we're returning is the same as the lifetime of the vector of rooms
//In the parent function, this makes it clear that you can't pass a vector of rooms to this function that the return value outlives
//We also need to tell the rust compiler the lifetime of current_room is as long as the vector of rooms because it's returned on failure to move

fn prompt_for_game_action<'a>(rooms: &'a Vec<Room>, current_room: &'a Room) -> Option<&'a Room> {
    //Prompt
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

    //Read input from the user
    handle.read_line(&mut buffer).unwrap();

    //Trim buffer to take whitespace off of the right-side of a string
    let buffer = buffer.trim_right();

    //Print a line
    println!("");
    //If our buffer now is "time", return None because we didn't enter a room name
    if buffer == "time" {
        return None;
    }

    //If buffer is a valid connection
    if current_room.connections.iter().any(|room_conn| room_conn == buffer) {
        //Loop over all the rooms and filter all the ones with wrong names. Then return the first item of the resulting iterator which should always be the room we
        return rooms.iter().filter(|room| room.name == buffer).next();
    }

    //If it's not in our list print "Ahh cmon guy" and return the same room we're in already
    println!("That is not a connection.\n");

    Some(current_room)
}

//Load the files from our directory
fn load_files<T: AsRef<Path>>(folder_dir: T) -> Result<Vec<Room>, Box<std::error::Error>> {
    let mut rooms: Vec<Room> = Vec::with_capacity(7);
    for entry in fs::read_dir(folder_dir)? {
        //? operates on `Result` or `Option`
        //Basically, it says "Hey if this result or option is Ok or Some, get that value. Otherwise, return the error"
        let file_name = entry?;
        //This line opens the filr and passes it to Room's `from` function to load and parse. There are two `?` here to return failures at each step
        rooms.push(Room::from(File::open(file_name.path())?)?)
    }

    Ok(rooms)
}

fn get_latest_directory() -> Option<PathBuf> {
    let dir_entries = fs::read_dir("./").unwrap();
    //For every file in the current directory
    dir_entries
        //Unwrap and get it's path as a string
        .map(|entry| entry.unwrap().path())
        //Filter the name to make sure it's the right format
        .filter(|file_name| {
            //Split out the `.` and compare skip the first item because `./blah` returns strings `` and `/blah`
            let file_split: Vec<&str> = file_name.to_str().unwrap().split(".").skip(1).collect();
            //Return true if it's length three and starts with `/sasol`
            file_split.len() == 3 && file_split[0] == "/sasol"
        })
        //Get the max based on some value
        .max_by_key(|file_name| {
            //Get the file metadata
            let meta = fs::metadata(file_name).unwrap();
            //Then read the last modified time and max by that result.
            meta.modified().unwrap()
        })
}
