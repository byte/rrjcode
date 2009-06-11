#!/bin/bash

while [ $# -ge 2 ]
do
    montage -geometry +0+00 "$1" "$2" "$1_and_$2"
    rm "$1" "$2"
    shift 2 
done
