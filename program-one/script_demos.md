# Readloop
```sh
$cat readloop
#!/bin/bash
while read myLine
do
    echo "$myLine"
done < $1
```
```sh
$ readloop test_file
1   1   1
1   3   4
5   3   1
```
```sh
$ cat test_file
1   1   1
1   3   4
5   3   1
```
---
# Arrayloop

*Note*: This is not the bash way of doing things. Prof has suggested not to use this.

```sh
#!/bin/bash
array=( one two three )
for i in "${array[@]}"
do
    echo $i
done
```
```sh
$ arrayloop
one
two
three
```
---
# Forloop
```sh
for i in list of items or an array
do
    commands
done
```
---
# Sumloop
```sh
#!/bin/bash
sum=0
TMP1=./tempfile
echo -e "8\n7\n6" > $TMP1

while read num
do
    echo "In Loop"
    echo "num: $num"
    sum=$(expr $sum + $num)
    echo "sum: $sum"
    echo -e "End of loop\n"
done < $TMP1
```

---
# stdin read
```sh
$ cat stdinread
#!/bin/bash
cat > "tempfile" # Waits here for input and routes it to a file
cat tempfile # Prints the tempfile
```
```sh
$ stdinread
I like cheese # This line is input we typed after the program is run
I like cheese # This is the line read from the `tempfile` that we just put into STDIN
```
```sh
$ cat test_file | stdinread
1   1   1
1   3   4
5   3   1
```
```sh
$ cat test_file
1   1   1
1   3   4
5   3   1
```
---
# Trtest

Three different examples of converting a single row of numbers into a single column of numbers

```sh
#!/bin/bash
inputFile="tempinputfile"
tempCol="tempcolfile"
tempRow="temprowfile"

# Make the input row file
echo -e "1\t2\t3\t4\t5" > $inputFile

# Append each number onto the end of a temporary file by cutting specific columns
# Skipping tab characters
cut -c 1 $inputFile > $tempCol
cut -c 3 $inputFile >> $tempCol
cut -c 5 $inputFile >> $tempCol
cut -c 7 $inputFile >> $tempCol
cut -c 9 $inputFile >> $tempCol

# Convert the column file back into a row file
cat $tempCol | tr '\n' '\t' > "$tempRow$$"

# Add a newline char to the end of the row file, for easier printing
echo >> "$tempRow$$"
```
