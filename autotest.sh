#!/bin/bash
if [ "$1" == "" ] || [ "$2" == "" ]; then 
    exit
fi
make
./Skyrmion test/testcase$1.txt benchmark/setting$2.txt test/output$1.dat test/nodeInfo$1.csv test/treeHeight$1.csv 2> test/log$1.log