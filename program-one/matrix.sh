#!/bin/bash

# $1 == operation
# $2 == m1file if any
# $3 == m2file if any

# $1 is the matrix to get a dim
dims () {
	rows=0

	while read line
	do
		latestline=$line
		rows=$(( $rows + 1 ))
	done < $1
	cols=`echo $latestline | grep -o " " | wc -l`
	cols=`expr $cols + 1`

	echo -e "$rows $cols"
}

transpose () {

	thedims=`dims $1`
	rows=`echo $thedims | cut -d " " -f 1`
	cols=`echo $thedims | cut -d " " -f 2`
	# echo Rows: $rows, Cols: $cols

	for colvar in `seq 1 $cols`
	do
		cutcol=`cut -f $colvar $1`
		# echo Colvar: $colvar, After Cut Cols: $cutcol
		for rowvar in `seq 1 $rows`
		do
			echo -n `echo $cutcol | cut -d " " -f $rowvar`
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

	for ((i=1;i<=$cutrows;i++))
	do
		lineone=`head -n $i < $1 | tail -n 1`
		linetwo=`head -n $i < $2 | tail -n 1`
		# echo "Lineone: $lineone"
		# echo "Linetwo: $linetwo"
		for ((x=1;x<=$cutcols;x++))
		do

			colone=`echo $lineone | cut -d " " -f $x`
			coltwo=`echo $linetwo | cut -d " " -f $x`
			echo -ne "$(( $colone + $coltwo ))"
			if [[ $x != $cutcols ]]
			then
				echo -ne "\t"
			fi
		done
		echo
	done
}

mult () {
	temp1=$1
	first=$2
	second=$temp1

	#echo First: $first, Second: $second
	#echo One: $1, Two: $2
	firstdims=`dims $first`
	firstrows=`echo $firstdims | cut -d " " -f 1`
	firstcols=`echo $firstdims | cut -d " " -f 2`
	# echo First dims: $firstdims, First Rows: $firstrows, First Cols: $firstcols

	seconddims=`dims $second`
	secondrows=`echo $seconddims | cut -d " " -f 1`
	secondcols=`echo $seconddims | cut -d " " -f 2`
	# echo Second dims: $seconddims, Second Rows: $secondrows, Second Cols: $secondcols
	transpose $second > secondtransposed

	for firstcol in `seq 1 $firstcols`
	do
		# lineone=`head -n $i < $1 | tail -n 1`
		# linetwo=`cat secondtransposed | head -n $i | tail -n 1`
		# echo "Lineone: $lineone"
		# echo "Linetwo: $linetwo"

		firstlinenums=`cat $first | cut -f $firstcol`
		#echo FirstLine: $firstlinenums

		for secondcol in `seq 1 $secondrows`
		do

			secondlinenums=`cat secondtransposed | cut -f $secondcol`
			#echo Second Line: $secondlinenums
			runningtotal=0
			for arow in `seq 1 $firstrows`
			do

				firstnum=`echo $firstlinenums | cut -d " " -f $arow`
				secondnum=`echo $secondlinenums | cut -d " " -f $arow`
				curval=$(( $firstnum * $secondnum ))
				runningtotal=$(( $runningtotal + $curval ))
			done
			echo -ne $runningtotal >> resultfile
			if [[ $secondcol != $secondrows ]]
			then
				echo -ne "\t" >> resultfile
			fi

			# colone=`echo $lineone | cut -d " " -f $x`
			# coltwo=`echo $linetwo | cut -d " " -f $x`
			# echo -ne "$(( $colone * $coltwo ))"
			# if [[ $x != $cutcols ]]
			# then
			#	echo -ne "\t"
			# fi
		done
		echo >> resultfile
	done

	transpose resultfile
	#cat resultfile
	rm secondtransposed
	rm resultfile
}



mean () {
	thedims=`dims $1`
	rows=`echo $thedims | cut -d " " -f 1`
	cols=`echo $thedims | cut -d " " -f 2`
	for colvar in `seq 1 $cols`
	do
		runningtotal=0
		for rowvar in `seq 1 $rows`
		do
			# echo Colvar: $colvar
			curval=`head -n $rowvar < $1 | tail -n 1 | cut -f $colvar`
			# echo Curval: $curval
			runningtotal=`expr $runningtotal + $curval`
		done
		# echo Running total: $runningtotal
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

	mean ${2:-/dev/stdin}
	exit 0
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
	dims2=`dims $3`
	dims2rev=`echo $dims2 | rev`


	# if [[ "$dims1" != "$dims2" && "$dims1" != "$dims2rev" ]]
	# then
	# 	echo "Cannot multiply these matricies" >&2
	# 	exit 3
	# fi

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

trap "rm secondtransposed && rm resultfile" SIGINT SIGTERM
