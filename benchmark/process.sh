#!/bin/bash

./Skyrmion ${1}5.in setting${2}.txt dat/${1}5_setting${2}_output.dat csv/${1}5_setting${2}_nodeInfo.csv csv/${1}5_setting${2}_Height.csv 2> log/${1}5_setting${2}_log.log &
./Skyrmion ${1}10.in setting${2}.txt dat/${1}10_setting${2}_output.dat csv/${1}10_setting${2}_nodeInfo.csv csv/${1}10_setting${2}_Height.csv 2> log/${1}10_setting${2}_log.log &
./Skyrmion ${1}50.in setting${2}.txt dat/${1}50_setting${2}_output.dat csv/${1}50_setting${2}_nodeInfo.csv csv/${1}50_setting${2}_Height.csv 2> log/${1}50_setting${2}_log.log &
./Skyrmion ${1}100.in setting${2}.txt dat/${1}100_setting${2}_output.dat csv/${1}100_setting${2}_nodeInfo.csv csv/${1}100_setting${2}_Height.csv 2> log/${1}100_setting${2}_log.log