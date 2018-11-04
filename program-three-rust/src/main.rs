use std::process::Command;
use std::io::{self, BufRead, BufReader, Read, Write};
use std::fs::{ReadDir, read_dir}

fn main() {

    loop {

        match prompt() {
            Some(command) if command == "cd" => {
                for entry in fs::read_dir() {

                }
            },
            Some(command) if command == "ls" => {

            },
            Some(command) => {

            },
            None => {

            }
        }

        if let None = prompt() {
            break;
        }

    }

}


fn prompt() -> Option<String> {
    println!("$");
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

    Some(buffer)
}
