use std::process::{Command, Child, id, Stdio};

pub struct ProcessPool {
    processes: Vec<Child>,
}

impl ProcessPool {

    pub fn new() -> Self {
        ProcessPool {
            processes: vec![]
        }
    }

    pub fn add(&mut self, command: &str, args: &[&str]) {

        let backgrounded = if let Some(arg) = args.get(args.len() - 1) {
            arg == &"&"
        } else {
            false
        };

        self.processes.push(
            Command::new(command)
                .args(args)
                .stdin(match backgrounded {
                    true => Stdio::null(),
                    false => Stdio::null(),
                })
                .spawn()
                .expect(&format!("Failed to run {}", command))
        );

    }

}
