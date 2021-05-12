#!/bin/bash
if [ "$1" == "" ] || [ "$2" == "" ]; then 
    exit
fi
make
./Skyrmion test/testcase$1.txt benchmark/setting/setting$2.txt testcase${1}_setting${2} test/ 2> test/log$1.log