#!/bin/bash

if [ "$1" == "" ] || [ "$2" == "" ]; then 
    exit
fi

echo time and memory > log/time.log

for setting in $(eval echo {$1..$2});
do
    echo setting: $setting

    echo tpch
    \time -v -a -o log/time.log ./process.sh tpch $setting

    echo zipfian
    \time -v -a -o log/time.log ./process.sh zipfian $setting

    echo uniform
    \time -v -a -o log/time.log ./process.sh uniform $setting

    echo hotspot
    \time -v -a -o log/time.log ./process.sh hotspot $setting

    echo sequential
    \time -v -a -o log/time.log ./process.sh sequential $setting

    echo exponential
    \time -v -a -o log/time.log ./process.sh exponential $setting

    echo latest
    \time -v -a -o log/time.log ./process.sh latest $setting
done