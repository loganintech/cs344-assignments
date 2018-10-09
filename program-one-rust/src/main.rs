use std::env;
use std::fs::File;
use std::io::{self, BufRead, BufReader, Read};
use std::process::exit;

// Type alias for matrix. Matrix is a 2-d array of 32-bit signed ints
type Matrix = Vec<Vec<i32>>;

//Loop over our matrix and print it from top to bottom, left to right
//Take it by reference because we don't want to steal ownership from the caller when printing
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

// Our main entrypoint. Returns a generic result with Box<Error> which is a smart pointer to the error type
// This means that we can return _any_ kind of error from thsi function. Should print the error body to stderr
fn main() -> std::result::Result<(), Box<std::error::Error>> {
    //Get an iterator of command-line arguments, skipping the program name
    //This is mutable because we use `next` to take values out of it
    let mut args = env::args().skip(1);

    //Grab the first param from the iterator.
    //.expect says "unwrap the value if it's Some(val), panic the program with the string otherwise"
    let func: String = args.next().expect("You must include a function argument.");

    //Grab the next param, which is a filename. Saves it as an option so we can check if it exists later
    let first_filename: Option<String> = args.next();
    //Same as above
    let second_filename: Option<String> = args.next();

    //func is type `String`, but we're matching `&'static str`. String.as_ref() is a &str, so we can match the pattern against that
    match func.as_ref() {
        "dims" => {
            // Rust lets you capture the resulting value of expressions. If statements are expressions and can work somewhat like a ternary operator
            // let some_val = if true { 3 } else { 5 };
            // some_val is the value of 3

            //If we have no first filename, load from stdin. If that fails, the program fails.
            let matrix = if first_filename.is_none() {
                parse_matrix(io::stdin())?
            } else {
                //Opens a file and unwraps the filename. This is safe because we already checked above that first_filename is not None
                parse_matrix(File::open(first_filename.unwrap())?)?
            };

            // second_filename should be `None` because we don't take two params for dims
            // error out if it does have something here
            if !second_filename.is_none() {
                eprintln!("You can only pass one file to dims.");
                exit(2);
            }

            // Get the dimensions of our matrix, passing the matrix by reference because we don't need to modify or own it
            let dims = dims(&matrix);
            // Print the dimensions to stdout
            println!("{} {}", dims[1][0], dims[0][0]);
        }
        "add" => {
            // Try to load a first filename, failing if the filename is None or matix cannot be parsed
            let matrix = parse_matrix(File::open(
                first_filename.expect("You must include a first matrix for this function."),
            )?)?;

            //Try to load a second filename
            let second_matrix = parse_matrix(File::open(
                second_filename.expect("You must include a second matrix for this function."),
            )?)?;

            if dims(&matrix) != dims(&second_matrix) {
                eprintln!("Cannot add matricies of different sizes.");
                exit(1);
            }

            print_matrix(&add_iterator(matrix, second_matrix));
        }
        "mean" => {
            //If we have no first filename, load from stdin. If that fails, the program fails.
            let matrix = if first_filename.is_none() {
                parse_matrix(io::stdin())?
            } else {
                //Opens a file and unwraps the filename. This is safe because we already checked above that first_filename is not None
                parse_matrix(File::open(first_filename.unwrap())?)?
            };

            print_matrix(&mean_iterator(&matrix));
        }
        "multiply" => {
            // Try to load a first filename, failing if the filename is None or matix cannot be parsed
            let matrix = parse_matrix(File::open(
                first_filename.expect("You must include a first matrix for this function."),
            )?)?;

            //Try to load a second filename
            let second_matrix = parse_matrix(File::open(
                second_filename.expect("You must include a second matrix for this function."),
            )?)?;

            if dims(&matrix)[0] != dims(&second_matrix)[1] {
                eprintln!("Cannot add matricies of different sizes.");
                exit(1);
            }

            print_matrix(&multiply_iterator(matrix, second_matrix));
        }
        "transpose" => {
            //If we have no first filename, load from stdin. If that fails, the program fails.
            let matrix = if first_filename.is_none() {
                parse_matrix(io::stdin())?
            } else {
                //Opens a file and unwraps the filename. This is safe because we already checked above that first_filename is not None
                parse_matrix(File::open(first_filename.unwrap())?)?
            };

            let transposed = transpose(matrix);
            print_matrix(&transposed);
        }
        //The case where none of the strings match
        _ => {
            //eprintln (error println) that it's not a valid command. Exit with 1
            eprintln!("That is not a valid command.");
            exit(1);
        }
    };

    //If we made it here, return Ok because the program ran successfully
    Ok(())
}

// vec! is a macro to create a new vector. This creates a vector of length 2 with first and second items being vectors of length 1 with the dimmensions
fn dims(matrix: &Matrix) -> Matrix {
    vec![vec![matrix.len() as i32], vec![matrix[0].len() as i32]]
}

//This takes the matrix by value, dropping the matrix at the end of the function
//After this function, the matrix passed into it by the calling code is unusable
//This is because of the way rust's borrow checker works. Read more here: https://doc.rust-lang.org/book/second-edition/ch04-00-understanding-ownership.html
//This could easily take matricies by reference if we wanted, preserving the original _and_ the transposed version
fn transpose(matrix: Matrix) -> Matrix {
    //We create a new, mutable matrix
    let mut transposed = Matrix::new();
    //Resize this matrix to be the exact capacity we need, filling in the values with correctly-sized columnns filled with 0
    //If a matrix were small enough to not need resizing, this will technically waste time, but will reduce vector resizing in the loop for large matricies
    //Overall this line was a design decision to try and make the function more consistently timed for large matricies
    //Recall that a Matrix's dimensions (AxB) becomes (BxA) when transposed
    transposed.resize(matrix[0].len(), vec![0; matrix.len()]);

    //Normally you'd use an iterator in this scenario, but I wanted my example code to look more C-like so we're doing C-style loops and matrix values
    for x_axis in 0..matrix.len() {
        for y_axis in 0..matrix[x_axis].len() {
            //Reflect the values across the axis
            transposed[y_axis][x_axis] = matrix[x_axis][y_axis];
        }
    }

    //Return the new matrix by value
    transposed
}

//Takes the matrix by reference so that we can keep the matrix and its means in the caller
#[allow(dead_code)]
fn mean(matrix: &Matrix) -> Matrix {
    //Create a new, mutable matrix
    let mut means: Matrix = Matrix::new();
    //Resize our new one to Ax1 as opposed to AxB size
    means.resize(matrix.len(), vec![0]);

    //Loop from 0 to the length of our matrix (exclusive)
    //0..3 loops over 0, 1, and 2
    for x_axis in 0..matrix.len() {
        for y_axis in 0..matrix[0].len() {
            //Collect the running total for each column in the first value of its column vector
            means[x_axis][0] += matrix[x_axis][y_axis];
        }
        //Divide that running total by the number of numbers, finding the mean. We have to cast to f32 so that we get a f32 result and round it like our program wants
        //Then we cast back to i32 so that the matrix structure is preserved
        means[x_axis][0] = (means[x_axis][0] as f32 / matrix[0].len() as f32).round() as i32;
    }

    //Return our means matrix
    means
}

//This is an idomatic rust implementation of the mean function above. It uses iterators instead of for loops with indexes
//Unlike the function above, this one cannot have an index out of bounds error nor can it have a divide-by-zero error.
fn mean_iterator(matrix: &Matrix) -> Matrix {
    //For every column in our matrix
    matrix
        //Iterate over each column
        .iter()
        //I did explicit type annotations here so we know it's a reference and can call methods like .sum
        .map(|item: &Vec<i32>| {
            //If the column has no values just return an empty vector
            if !item.len() > 0 {
                return vec![];
            }

            //Otherwise, return a single-length vector with the sum divded by the length, rounded
            vec![(item.iter().sum::<i32>() as f32 / item.len() as f32).round() as i32]
        })
        //Then collect all of the columns into our matrix
        .collect()
}

//This function also consumes its calling matricies by value. This could also be by reference, but after multiplying it'll drop and that preserves memory (although not much)
#[allow(dead_code)]
fn multiply(matrix: Matrix, second_matrix: Matrix) -> Matrix {
    let mut result = Matrix::new();
    //Set the values of this vector to 1 because multiplying by 0 is always 0
    result.resize(second_matrix.len(), vec![1; matrix[0].len()]);

    //Transpose our first matrix
    let transposed = transpose(matrix);

    //The loop bellow collects the multiplication sums into our result matrix. Nothing fancy happens here
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

//This is an idomatic rust implementation of the multiply function above. It uses iterators instead of for loops with indexes
//Unlike the function above, this one cannot have an index out of bounds error and therefore cannot panic and crash the program
fn multiply_iterator(matrix: Matrix, second_matrix: Matrix) -> Matrix {
    let transposed = transpose(matrix);

    second_matrix
        //Iterate over our second matrix
        .iter()
        //For each column in the first matrix
        .map(|column| {
            transposed
                //Iterate over all the columns in the second
                .iter()
                .map(|second_column| {
                    //For each of the first columns
                    column
                        //Iterate over the values in it
                        .iter()
                        //Zip the values from the second column
                        .zip(second_column)
                        //Starting from zero, set the accumulator value to the value of the accumulator + the (first * second) element
                        .fold(0, |accum, (first_elem, second_elem)| {
                            accum + (first_elem * second_elem)
                        }) //We don't need to `collect` this one because we're getting a single value from our iterator and returning that to the map
                })
                //Now we collect all the values for that column
                .collect()
        })
        //And we collect all the columns together into one matrix
        .collect()
}

#[allow(dead_code)]
fn add(matrix: Matrix, second_matrix: Matrix) -> Matrix {
    let mut result = Matrix::new();
    //Our matrix should be the same size as the source matrix, so we resize to that
    result.resize(matrix.len(), vec![0; matrix[0].len()]);

    //Loop from 0..matrix.len() and add the values in each position to our result matrix
    for x_axis in 0..matrix.len() {
        for y_axis in 0..matrix[0].len() {
            result[x_axis][y_axis] = matrix[x_axis][y_axis] + second_matrix[x_axis][y_axis];
        }
    }

    result
}

//This is an idomatic rust implementation of the add function above. It uses iterators instead of for loops with indexes
//Unlike the function above, this one cannot have an index out of bounds error and therefore cannot panic and crash the program
fn add_iterator(matrix: Matrix, second_matrix: Matrix) -> Matrix {
    matrix
        //Convert matrix to an iter
        .iter()
        //Zip the second matrix's iter into it, so they iterate together
        .zip(second_matrix.iter())
        //The zip creates a tuple with left being the first matrix and right being the second
        .map(|(left_col, right_col)| {
            left_col
                //Iterate over the first column from `matrix`
                .iter()
                //Zip the right column into the left so we're iterating over values together
                .zip(right_col.iter())
                //For each row, put the left and right values together
                .map(|(left_val, right_val)| left_val + right_val)
                //Collect them into a vector of our added values
                .collect()
        })
        //Collect all of our columns into one vector of matricies
        .collect()
}

//This function takes a generic parameter
//You can read it like this
//
// For all types `T` where `T` implements the Trait `Read`, parse_matrix for that T, taking T by value
fn parse_matrix<T: Read>(source: T) -> io::Result<Matrix> {
    //Create a buffered reader of type `T` from our source.
    //This doesn't help all that much for stdin, but should decrease the time we spend waiting for the operating system to return lines from a file
    let source: BufReader<T> = BufReader::new(source);
    let mut matrix: Matrix = Matrix::new();

    //For every line in our source
    for line in source.lines() {
        //Use ? to see if line is `Ok`, if not we return whatever the `Err` value is (since we use io::Result<Matrix>)
        let line = line?;
        //Get the numbers out of the line
        let line_numbers: Vec<i32> = line
            //split on tabs
            .split("\t")
            //For each item in our parsed list, have that value run against a closeure `|param_one, param_two, &param_three, etc...| {}`
            //In this case, for any x try to parse x as an i32. If it doesn't work, unwrap() panics. This is because our input is not the correct type
            //We could also do a match in here to check if it's Ok or Err and provide default values if we chose to
            .map(|x| x.parse::<i32>().unwrap())
            //Then collect this iterator into our vector
            //If we don't run `collect` this just becomes an iterator with adapters
            //Iterators in Rust are lazy, meaning they don't manipilate any values until they're collected or iterated over
            .collect();

        //If the value was nothing, resize to nothing.
        if matrix.len() == 0 {
            matrix.resize(line_numbers.len(), Vec::new());
        }

        line_numbers
            //This creates an iterator that takes values (the `into` part shows this) meaning that after this line, the vector `line_numers` has been moved and cannot be accessed
            .into_iter()
            //Enumerate the index
            .enumerate()
            //For every value returned by the iterator, take the enumeration as y_axis and the value as val. The enumeration corresponds with our column, or the y axis
            //So we index our matrix and say 'matrix at [y_axis] .push(val)` or in other words, for each column add the line's number to the bottom of the column
            .for_each(|(y_axis, val)| {
                matrix[y_axis].push(val);
            });
    }

    //Return Ok(matrix) so that the calling function knows that the operation was successful and returns the matrix we need.
    Ok(matrix)
}
