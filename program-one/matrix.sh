#!/bin/bash

# $1 == operation
# $2 == m1file if any
# $3 == m2file if any

# $1 is the matrix to get a dim
dims () {
	rows=0

	#read all the lines in a row
	while read line
	do
		latestline=$line
		# incr the number of rows
		rows=$(( $rows + 1 ))
	done < $1
	#once we get the last line see how many lines there are when "grep" ing spaces
	cols=`echo $latestline | grep -o " " | wc -l`
	cols=`expr $cols + 1`

	echo -e "$rows $cols"
}

transpose () {

	# get our dims
	thedims=`dims $1`
	rows=`echo $thedims | cut -d " " -f 1`
	cols=`echo $thedims | cut -d " " -f 2`

	for colvar in `seq 1 $cols`
	do
		# cut out the column we're on
		cutcol=`cut -f $colvar $1`
		for rowvar in `seq 1 $rows`
		do
			# paste each val horizontally
			echo -n `echo $cutcol | cut -d " " -f $rowvar`
			# if it's not the last row add a tab
			if [[ $rowvar != $rows ]]
			then
				echo -ne "\t"
			fi
		done
		echo
	done

}

# $1 is the first matrix
# $2 is the second matrix
add () {
	thedims=$(dims $1)
	cutcols=`echo $thedims | cut -d " " -f 2`
	cutrows=`echo $thedims | cut -d " " -f 1`

	# for each row
	for ((i=1;i<=$cutrows;i++))
	do
		# get the i'th line from each file
		lineone=`head -n $i < $1 | tail -n 1`
		linetwo=`head -n $i < $2 | tail -n 1`
		for ((x=1;x<=$cutcols;x++))
		do

			# pick out the numbers we're adding together
			colone=`echo $lineone | cut -d " " -f $x`
			coltwo=`echo $linetwo | cut -d " " -f $x`
			# add and print them
			echo -ne "$(( $colone + $coltwo ))"

			#add a tab if not the last column
			if [[ $x != $cutcols ]]
			then
				echo -ne "\t"
			fi
		done
		echo
	done
}

mult () {
	#swap the input orders
	temp1=$1
	first=$2
	second=$temp1
	# get the dims of the first
	firstdims=`dims $first`
	firstrows=`echo $firstdims | cut -d " " -f 1`
	firstcols=`echo $firstdims | cut -d " " -f 2`

	# get the dims of the second
	seconddims=`dims $second`
	secondrows=`echo $seconddims | cut -d " " -f 1`
	secondcols=`echo $seconddims | cut -d " " -f 2`
	transpose $second > secondtransposed

	# loop over the first columns
	for firstcol in `seq 1 $firstcols`
	do
		# grab the first column
		firstlinenums=`cat $first | cut -f $firstcol`

		#loop over the second rows (but after transposed, is actually the columns size)
		for secondcol in `seq 1 $secondrows`
		do

			#get the column we're multiplying
			secondlinenums=`cat secondtransposed | cut -f $secondcol`
			# make a running total var
			runningtotal=0
			for arow in `seq 1 $firstrows`
			do

				# for every row multiply the numbers
				firstnum=`echo $firstlinenums | cut -d " " -f $arow`
				secondnum=`echo $secondlinenums | cut -d " " -f $arow`
				curval=$(( $firstnum * $secondnum ))
				runningtotal=$(( $runningtotal + $curval ))
			done
			# add the running total to our result file
			echo -ne $runningtotal >> resultfile
			if [[ $secondcol != $secondrows ]]
			then
				echo -ne "\t" >> resultfile
			fi

		done
		echo >> resultfile
	done

	#transpose the result file so that we have the proper output structure
	transpose resultfile
	rm secondtransposed
	rm resultfile
}



mean () {
	#grab our dims
	thedims=`dims $1`
	rows=`echo $thedims | cut -d " " -f 1`
	cols=`echo $thedims | cut -d " " -f 2`


	# for each col
	for colvar in `seq 1 $cols`
	do
		# loop over the number row by row
		runningtotal=0
		for rowvar in `seq 1 $rows`
		do
			# cut out our number
			curval=`head -n $rowvar < $1 | tail -n 1 | cut -f $colvar`
			# add our number to the total
			runningtotal=`expr $runningtotal + $curval`
		done
		# get the average. Add half the row count so that things will "round up" when truncating
		resultval="$(( $(( $runningtotal + $(($rows/2)) )) / $rows))"
		if [[ $resultval -lt 0 ]]
		then
			resultval=`expr $resultval - 1`
		fi
		echo -n $resultval
		if [[ $colvar != $cols ]]
		then
			echo -ne "\t"
		fi
	done
	echo
}

if [[ "$1" = "dims" ]]
then
	# If we don't have two params or don't have one param w/  stdin
	if [[ !( ("$#" -eq 1 && -n ${-/dev/stdin}) || "$#" -eq 2) ]]
	then
		echo "Bad params" >&2
		exit 1
	fi

	#if file doesn't exist and (we only have one val and stdin doesn't exist)
	if [[ ! (-f $2 || ("$#" -eq 1 && -n ${-/dev/stdin})) ]]
	then
		echo "File not found" >&2
		exit 2
	fi

	dims ${2:-/dev/stdin}
	exit 0
elif [ "$1" = "add" ]
then

	# if we have the wrong number of params
	if [[ !("$#" -eq 3) ]]
	then
		echo "Bad params" >&2
		exit 1
	fi

	# if we don't have either file
	if [[ !(-f $2 || -f $3) ]]
	then
		echo "File not found" >&2
		exit 2
	fi

	dims1=`dims $2`
	dims2=`dims $3`

	if [[ "$dims1" != "$dims2" ]]
	then
		echo "Matricy size mismatch" >&2
		exit 3
	fi

	add $2 $3
	exit 0
elif [ "$1" = "mean" ]
then
	# If we don't have two params or don't have one param w/  stdin
	if [[ !( ("$#" -eq 1 && -n ${-/dev/stdin}) || "$#" -eq 2) ]]
	then
		echo "Bad params" >&2
		exit 1
	fi

	#if file doesn't exist and (we only have one val and stdin doesn't exist)
	if [[ ! (-f $2 || ("$#" -eq 1 && -n ${-/dev/stdin})) ]]
	then
		echo "File not found" >&2
		exit 2
	fi

	mean ${2:-/dev/stdin}
	exit 0
elif [ "$1" = "multiply" ]
then

	# if we have wrong number of params
	if [[ !("$#" -eq 3) ]]
	then
		echo "Bad params" >&2
		exit 1
	fi

	#if either file doesn't exist
	if [[ !(-f $2 || -f $3) ]]
	then
		echo "File not found" >&2
		exit 2
	fi

	mult $2 $3
	exit 0
elif [ "$1" = "transpose" ]
then
	# If we don't have two params or don't have one param w/  stdin
	if [[ !"$#" -eq 2 ]]
	then
		echo "Bad params" >&2
		exit 1
	fi

	if [[ !(-f $2)  ]]
	then
		echo "File not found" >&2
		exit 2
	fi

	if [[ ! (-r $2) ]]
	then
		echo "File cannot be read" >&2
		exit 3
	fi

	transpose $2
	exit 0
else
	echo "That is not a valid command." >&2
	exit 1
fi
