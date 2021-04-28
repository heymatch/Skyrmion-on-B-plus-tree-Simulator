#!/bin/bash
if [ "$1" == "" ] || [ "$2" == "" ]; then 
    exit
fi
make
./Skyrmion.out test/testcase$1.txt test/setting$2.txt test/output$1.dat 2> test/log$1.log