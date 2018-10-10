extern crate rand;

use std::fs::File;

use rand::prelude::*;

fn main() {
    let mut rng = thread_rng();

    let fileone = File::open("fileone.txt").unwrap();
    let fieltwo = File::open("filetwo.txt").unwrap();
    let filethree = File::open("filethree.txt").unwrap();



}
