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

# mult () {

# }

transpose () {

    thedims=`dims $1`
    cols=`dims $1 | cut -c 3`
    # echo $cols

    for ((i=1;i<=$cols;i++))
    do
        aftercut=`cut -f$i $1`
        # echo Aftercut: $aftercut
        afterpaste=`echo $aftercut | paste -s`
        # echo Afterpaste: $afterpaste

        echo $afterpaste

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
    dims2=`dims $3`

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

