use std::path::PathBuf;

pub fn status(cwd: &PathBuf) {
    println!("CWD: {}", cwd.to_str().unwrap());
}
