extern crate dirs;

use std::process::Command;
use std::io::{self, BufRead, BufReader, Read, Write};
use std::fs::{ReadDir, read_dir};

use std::env::{current_dir};
use std::path::{Path, PathBuf};

mod builtins;


fn main() -> Result<(), Box<std::error::Error>> {

    let mut current_path = current_dir()?;

    loop {

        let command = prompt().unwrap();
        let mut command_parts = command.split_whitespace();
        let program = command_parts.next().unwrap();

        match program {
            "cd" => {
                if let Some(new_path) = builtins::cd::change_directory(command_parts.next()) {
                    current_path = new_path;
                }
            }
            "status" => {
                builtins::status::status(&current_path);
            }
            //Run arbitrary command
            _ => {

            }

        }
 

    }

    Ok(())
}


fn prompt() -> Option<String> {
    print!(": ");
    io::stdout().flush().unwrap();

    let mut buffer = String::new();
    let stdin = io::stdin();
    let mut handle = stdin.lock();

    //Read input from the user
    handle.read_line(&mut buffer).unwrap();

    //Trim buffer to take whitespace off of the right-side of a string
    let buffer = buffer.trim_right();

    Some(buffer.into())
}
