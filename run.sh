#!/bin/bash

#for i in {1..10}
#do
#  (
#    cd /home/sofia/Projects/CloudDB/pool-hashjoin-pcm/build
#    sudo ./Pool-HashJoin-PCM --r-size=2000000000 --s-size=2000000000 --total-cores=14 --task-size=10000000 --skew=0
#  ) &
#  (
#    cd /home/sofia/Projects/CloudDB/busy-cores/build
#    sudo ./Busy-Cores 2 3 4
#  ) &
#
#  wait
##  cd ..
#  cd /home/sofia/Projects/CloudDB/pool-hashjoin-pcm
#  ./plot.sh
#done
#sudo ./Pool-HashJoin-PCM --r-size=200 --s-size=200 --total-cores=14 --task-size=10 --skew=1
#sudo ./Pool-HashJoin-PCM --r-size=2000000000 --s-size=2000000000 --total-cores=14 --task-size=10000000 --skew=0
#cd ..
#./plot.sh

#cd build
#sudo ./Pool-HashJoin-PCM --r-size=100 --s-size=100 --total-cores=14 --task-size=10 --skew=0


for i in {1..10}
do
  cd build
  sudo ./Pool-HashJoin-PCM --r-size=2000000000 --s-size=2000000000 --total-cores=14 --task-size=10000000 --skew=0

  cd ..
  ./plot.sh
done
