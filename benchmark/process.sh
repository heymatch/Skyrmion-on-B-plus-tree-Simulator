#!/bin/bash

./Skyrmion workload/${1}5.in setting/setting${2}.txt ${1}5_setting${2} out/ 2> log/${1}5_setting${2}_log.log &
./Skyrmion workload/${1}10.in setting/setting${2}.txt ${1}10_setting${2} out/ 2> log/${1}10_setting${2}_log.log &
./Skyrmion workload/${1}50.in setting/setting${2}.txt ${1}50_setting${2} out/ 2> log/${1}50_setting${2}_log.log &
./Skyrmion workload/${1}100.in setting/setting${2}.txt ${1}100_setting${2}_output.dat out/ 2> log/${1}100_setting${2}_log.log &

wait