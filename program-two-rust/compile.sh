#!/bin/bash

cargoexists=`command -v cargo`
if [ ! -n $cargoexists ]
then

    echo You need to install the rust toolchain at https://rustup.rs
    exit 1

fi

make
