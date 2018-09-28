#!/bin/bash

# $1 == operation
# $2 == m1file if any
# $3 == m2file if any

if [ "$1" = "dims" ]
then

    lines=0

    while read line
    do
        echo "Line: $line"
        lines=`expr $lines + 1`
    done < ${2:-/dev/stdin}

    cols=`awk '{print NF}' $line | sort -nu | tail -n 1`

    echo "$lines $cols"
fi
