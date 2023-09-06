#!/bin/bash

cd build
sudo ./Pool-HashJoin-PCM --r-size=800000 --s-size=800000 --total-cores=14 --task-size=10 & (
  cd /home/sofia/Projects/CloudDB/busy-cores/build
  sudo ./Busy-Cores 2 3 4
)

cd /home/sofia/Projects/CloudDB/pool-hashjoin-pcm
./plot.sh