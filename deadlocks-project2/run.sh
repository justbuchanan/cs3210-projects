#!/bin/sh

cd lock_kmod && make load && cd ..
export LD_LIBRARY_PATH=$(pwd):$LD_LIBRARY_PATH
./bestoffer

