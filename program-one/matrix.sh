#!/bin/bash


MATRIXONE=()
MATRIXTWO=()
read_file () {
    index=0
    while IFS=$'' read -r line; do
        # echo "Reading Line"
        local numbers=()

        # echo "Text read from file: ${line}"

        echo $line | while IFS=$' ' read -r -a numbers; do
            # echo "Reading into Array"
            innerindex=0
            for num in "${numbers[@]}"
            do
                echo "Num: $num"
                echo "Inner Index: $innerindex"
                innerindex=`expr $innerindex + 1`
                MATRIXONE["$index","$innerindex"]="$num"
                # echo "Index: $index"
                echo "M1: ${MATRIXONE[$index,$innerindex]}"

            done
        done
        index=`expr $index + 1`

    done < "$1"
}

print_matrix_one () {

    rows=${#MATRIXONE[@]}
    echo "Rows: $rows"
    rowcount=0
    until [ $rowcount -ge $rows ];
    do
        cols=${#MATRIXONE[$rowcount,@]}
        colcount=0
        until [ $colcount -ge $cols ];
        do

            colcount=`expr $colcount + 1`
        done

        rowcount=`expr $rowcount + 1`

    done

}

read_file $1 $2

print_matrix_one
