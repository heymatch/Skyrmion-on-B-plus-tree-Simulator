#!/bin/bash

if [ "$1" == "" ] || [ "$2" == "" ]; then 
    exit
fi

echo time and memory > log/time.log

for setting in $(eval echo {$1..$2});
do
    echo setting: $setting

    echo tpch
    bash process.sh tpch $setting

    echo zipfian
    bash process.sh zipfian $setting

    echo uniform
    bash process.sh uniform $setting

    echo hotspot
    bash process.sh hotspot $setting

    echo sequential
    bash process.sh sequential $setting

    echo exponential
    bash process.sh exponential $setting

    echo latest
    bash process.sh latest $setting
done