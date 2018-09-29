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
    cols=`awk '{print NF}' <<< $latestline | sort -nu | tail -n 1`

    echo -e "$lines $cols"
}

# $1 is the first matrix
# $2 is the second matrix
add () {
    thedims=$(dims $1)
    cutrows=`echo $thedims | cut -c 1`
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
    thedims=$(dims $1)
    cutrows=`echo $thedims | cut -c 1`
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
            echo -n -e "$(( $colone * $coltwo ))\t"
        done
        echo ""
    done
}

if [[ "$1" = "dims" && ( ("$#" -eq 1 && -n ${-/dev/stdin}) || "$#" -eq "2") ]]
then
    dims ${2:-/dev/stdin}
elif [ "$1" = "add" ]
then
    # echo "Add"
    add $2 $3
    exit 0
elif [ "$1" = "mean" ]
then
    echo "Mean"
    # mean ${2:-/dev/stdin}
elif [ "$1" = "multiply" ]
then
    # echo "Mult"
    mult $2 $3
    exit 0
elif [ "$1" = "transpose" ]
then
    echo "Transpose"
else
    echo "That is not a valid command."
fi

