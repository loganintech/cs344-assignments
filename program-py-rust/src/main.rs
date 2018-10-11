extern crate rand;

use std::fs::File;
use std::io::Write;

use rand::prelude::*;
use rand::distributions::Alphanumeric;

fn main() {
    let mut rng = thread_rng();

    let fileone = File::create("fileone.txt").unwrap();
    let filetwo = File::create("filetwo.txt").unwrap();
    let filethree = File::create("filethree.txt").unwrap();

    let charsone = rng.sample_iter(&Alphanumeric)
                      .filter(|achar: &char| achar.is_ascii_alphabetic())
                      .take(10)
                      .collect::<String>();

    fileone.write(charsone);

    let first: u64 = rng.next_u32() as u64;
    let second: u64 = rng.next_u32() as u64;
    let product: u64 = first * second;

    println!("{}\n{}\n{}", first, second, product);

}
