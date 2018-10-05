#!/bin/bash

# $1 == operation
# $2 == m1file if any
# $3 == m2file if any

# $1 is the matrix to get a dim
dims () {
	lines=0

	while read line
	do
		latestline=$line
		lines=$(( $lines + 1 ))
	done < $1

	cols=`echo $latestline | grep -o " " | wc -l`
	cols=`expr $cols + 1`

	echo -e "$lines $cols"
}

# $1 is the first matrix
# $2 is the second matrix
add () {
	thedims=$(dims $1)
	cutcols=`echo $thedims | cut -c 3`

	for ((i=1;i<$cutcols;i++))
	do
		lineone=`head -n $i < $1 | tail -n 1`
		linetwo=`head -n $i < $2 | tail -n 1`
		# echo "Lineone: $lineone"
		# echo "Linetwo: $linetwo"
		for ((x=1;x<$cutcols*2;x+=2))
		do

			colone=`echo $lineone | cut -c $x`
			coltwo=`echo $linetwo | cut -c $x`
			echo -n -e "$(( $colone + $coltwo ))\t"
		done
		echo ""
	done
}

mult () {
	firstdims=`dims $1`
	firstrows=`echo $firstdims | cut -c 1`
	firstcols=`echo $firstdims | cut -c 3`
	# echo First dims: $firstdims, First Rows: $firstrows, First Cols: $firstcols

	seconddims=`dims $2`
	secondrows=`echo $seconddims | cut -c 1`
	secondcols=`echo $seconddims | cut -c 3`
	# echo Second dims: $seconddims, Second Rows: $secondrows, Second Cols: $secondcols


	# For every row in the first matrix
	for ((i=1;i<=$firstrows;i++))
	do
		# Get a column from the second
		for ((x=1;x<=$secondcols;x++))
		do
			runningtotal=0
			#Go down each number from the second
			for ((j=1;j<=$firstcols;j++))
			do
				# echo First Row Index: $i
				# echo Second Col Index: $x
				# echo Second Row Index: $j
				firstafterheadtail=`head -n $i < $1 | tail -n 1`
				# echo First After Head/Tail $firstafterheadtail
				firstnum=`echo $firstafterheadtail | cut -d " " -f $j`

				secondafterheadtail=`head -n $j < $2 | tail -n 1`
				# echo After Head/Tail: $secondafterheadtail
				secondnum=`echo $secondafterheadtail | cut -d " " -f $x`

				# echo Second Num after Cut: $secondnum
				# echo First Num: $firstnum
				# echo Second Num: $secondnum
				# echo Multiplying $firstnum and $secondnum
				numsum=$(($firstnum * $secondnum))
				runningtotal=`expr $runningtotal + $numsum`
			done
			echo -n -e "$runningtotal"
			#Don't want to add a tab at the end
			if [[ $x != $firstcols ]]
			then
				echo -n -e "\t"
			fi
		done
		echo ""
	done

}

transpose () {

	thedims=`dims $1`
	cols=`dims $1 | cut -d " " -f 2`

	for colvar in `seq 1 $cols`
	do
		aftercut=`cut -d'	' -f$colvar $1`
		echo Aftercut: $aftercut | cat -A
		afterpaste=`echo $aftercut | paste -s`
		# echo Afterpaste: $afterpaste

		echo $aftercut

		# echo -n $afterpaste

		# if [[ ! ("$i" -eq $cols) ]]
		# then
		#     echo ""
		# fi

	done

}

if [[ "$1" = "dims" ]]
then
	# If we don't have two params or don't have one param w/  stdin
	if [[ !( ("$#" -eq 1 && -n ${-/dev/stdin}) || "$#" -eq 2) ]]
	then
		echo "Bad params" >&2
		exit 1
	fi

	if [[ ! (-f $2 || ("$#" -eq 1 && -n ${-/dev/stdin})) ]]
	then
		echo "File not found" >&2
		exit 2
	fi

	dims ${2:-/dev/stdin}
	exit 0
elif [ "$1" = "add" ]
then

	if [[ !("$#" -eq 3) ]]
	then
		echo "Bad params" >&2
		exit 1
	fi

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
	echo "Mean"
	# mean ${2:-/dev/stdin}
elif [ "$1" = "multiply" ]
then
	if [[ !("$#" -eq 3) ]]
	then
		echo "Bad params" >&2
		exit 1
	fi

	if [[ !(-f $2 || -f $3) ]]
	then
		echo "File not found" >&2
		exit 2
	fi

	dims1=`dims $2`
	dims2=`dims $3 | rev`

	if [[ "$dims1" != "$dims2" ]]
	then
		echo "Cannot multiply these matricies" >&2
		exit 3
	fi
	# echo "Mult"
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

