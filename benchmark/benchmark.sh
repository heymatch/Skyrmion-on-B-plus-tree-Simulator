#!/bin/bash

for setting in {1..16};
do
    echo setting: $setting

    echo tpch
    ./process.sh tpch $setting

    echo zipfian
    ./process.sh zipfian $setting

    echo uniform
    ./process.sh uniform $setting

    echo hotspot
    ./process.sh hotspot $setting

    echo sequential
    ./process.sh sequential $setting

    echo exponential
    ./process.sh exponential $setting

    echo latest
    ./process.sh latest $setting
done