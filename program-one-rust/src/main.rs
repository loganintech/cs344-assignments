use std::env;
use std::fs::File;
use std::io::{self, BufRead, BufReader, Read};
use std::process::exit;

type Matrix = Vec<Vec<i32>>;

fn print_matrix(matrix: &Matrix) {
    for y_axis in 0..matrix[0].len() {
        for x_axis in 0..matrix.len() {
            print!("{}", matrix[x_axis][y_axis]);
            if x_axis != matrix.len() - 1 {
                print!("\t");
            }
        }

        println!("");
    }
}

fn main() -> std::result::Result<(), Box<std::error::Error>> {
    let mut args = env::args().skip(1);

    let func: String = match args.next() {
        Some(f) => f,
        None => {
            eprintln!("You must include a function argument.");
            exit(1);
        }
    };

    let first_filename: Option<String> = args.next();
    let second_filename: Option<String> = args.next();

    match func.as_ref() {
        "dims" => {
            let matrix = if first_filename.is_none() {
                get_stdin()?
            } else {
                match first_filename {
                    Some(filename) => get_file(filename.as_ref())?,
                    None => {
                        eprintln!("You need to include a filename or something to stdin.");
                        exit(1);
                    }
                }
            };

            if !second_filename.is_none() {
                eprintln!("You can only pass one file to dims.");
                exit(2);
            }

            let dims = dims(&matrix);
            println!("{} {}", dims[1][0], dims[0][0]);
            return Ok(());
        }
        "add" => {
            let matrix = if first_filename.is_none() {
                get_stdin()?
            } else {
                match first_filename {
                    Some(filename) => get_file(filename.as_ref())?,
                    None => {
                        eprintln!("You need to include a filename or something to stdin.");
                        exit(1);
                    }
                }
            };

            let second_matrix = match second_filename {
                Some(filename) => get_file(filename.as_ref())?,
                None => {
                    eprintln!("You need to include a filename or something to stdin.");
                    exit(1);
                }
            };

            if dims(&matrix) != dims(&second_matrix) {
                eprintln!("Cannot add matricies of different sizes.");
                exit(1);
            }

            print_matrix(&add(matrix, second_matrix));
            return Ok(());
        }
        "mean" => {
            let matrix = if first_filename.is_none() {
                get_stdin()?
            } else {
                match first_filename {
                    Some(filename) => get_file(filename.as_ref())?,
                    None => {
                        eprintln!("You need to include a filename or something to stdin.");
                        exit(1);
                    }
                }
            };

            print_matrix(&mean(matrix));
            return Ok(());
        }
        "multiply" => {
            let matrix = if first_filename.is_none() {
                get_stdin()?
            } else {
                match first_filename {
                    Some(filename) => get_file(filename.as_ref())?,
                    None => {
                        eprintln!("You need to include a filename or something to stdin.");
                        exit(1);
                    }
                }
            };

            let second_matrix = match second_filename {
                Some(filename) => get_file(filename.as_ref())?,
                None => {
                    eprintln!("You need to include a filename or something to stdin.");
                    exit(1);
                }
            };

            print_matrix(&multiply(matrix, second_matrix));
            return Ok(());
        }
        "transpose" => {
            let matrix = if first_filename.is_none() {
                get_stdin()?
            } else {
                match first_filename {
                    Some(filename) => get_file(filename.as_ref())?,
                    None => {
                        eprintln!("You need to include a filename or something to stdin.");
                        exit(1);
                    }
                }
            };

            let transposed = transpose(matrix);
            print_matrix(&transposed);
            return Ok(());
        }
        _ => {
            eprintln!("That is not a valid command.");
            exit(1);
        }
    }
}

fn dims(matrix: &Matrix) -> Matrix {
    vec![vec![matrix.len() as i32], vec![matrix[0].len() as i32]]
}

fn transpose(matrix: Matrix) -> Matrix {
    let mut transposed = Matrix::new();
    transposed.resize(matrix[0].len(), vec![0; matrix.len()]);

    for x_axis in 0..matrix.len() {
        for y_axis in 0..matrix[x_axis].len() {
            transposed[y_axis][x_axis] = matrix[x_axis][y_axis];
        }
    }

    transposed
}

fn mean(matrix: Matrix) -> Matrix {
    let mut means: Matrix = Matrix::new();
    means.resize(matrix.len(), Vec::new());

    for x_axis in 0..matrix.len() {
        means[x_axis].push(0);
        for y_axis in 0..matrix[0].len() {
            means[x_axis][0] += matrix[x_axis][y_axis];
        }
        means[x_axis][0] = (means[x_axis][0] as f32 / matrix[0].len() as f32).round() as i32;
    }

    means
}

fn multiply(matrix: Matrix, second_matrix: Matrix) -> Matrix {
    let mut result = Matrix::new();
    result.resize(second_matrix.len(), vec![1; matrix[0].len()]);

    let transposed = transpose(matrix);

    for first_x in 0..second_matrix.len() {
        for second_x in 0..transposed.len() {
            let mut total = 0;
            for element in 0..second_matrix[first_x].len() {
                total += second_matrix[first_x][element] * transposed[second_x][element];
            }
            result[first_x][second_x] = total;
        }
    }

    result
}

fn add(matrix: Matrix, second_matrix: Matrix) -> Matrix {
    let mut result = Matrix::new();
    result.resize(matrix.len(), vec![1; matrix[0].len()]);

    for x_axis in 0..matrix.len() {
        for y_axis in 0..matrix[0].len() {

            result[x_axis][y_axis] = matrix[x_axis][y_axis] + second_matrix[x_axis][y_axis];

        }
    }

    result
}

fn get_stdin() -> io::Result<Matrix> {
    parse_matrix(io::stdin())
}

fn get_file(filename: &str) -> io::Result<Matrix> {
    parse_matrix(File::open(filename)?)
}

fn parse_matrix<T: Read>(source: T) -> io::Result<Matrix> {
    let source: BufReader<T> = BufReader::new(source);
    let mut matrix: Matrix = Matrix::new();

    for line in source.lines() {
        let line = line?;
        let line_numbers: Vec<i32> = line
            .split("\t")
            .map(|x| x.parse::<i32>().unwrap())
            .collect();

        if matrix.len() == 0 {
            matrix.resize(line_numbers.len(), Vec::new());
        }

        line_numbers
            .into_iter()
            .enumerate()
            .for_each(|(y_axis, val)| {
                matrix[y_axis].push(val);
            });
    }

    Ok(matrix)
}
