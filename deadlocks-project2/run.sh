#!/bin/sh

export LD_LIBRARY_PATH=$(pwd):$LD_LIBRARY_PATH
make clean
make all
cd lock_kmod
make load
cd ../
./bestoffer

