#!/bin/bash


for i in {1..10}
do
  cd build
#  sudo ./Pool-HashJoin-PCM --r-size=20000 --s-size=20000 --total-cores=14 --task-size=1000 --skew=0
  sudo ./Pool-HashJoin-PCM --r-size=2000000000 --s-size=2000000000 --total-cores=14 --task-size=10000000 --skew=0
  cd ..
  ./plot.sh
done
#sudo ./Pool-HashJoin-PCM --r-size=200 --s-size=200 --total-cores=14 --task-size=10 --skew=1
#sudo ./Pool-HashJoin-PCM --r-size=2000000000 --s-size=2000000000 --total-cores=14 --task-size=10000000 --skew=0
#cd ..
#./plot.sh
