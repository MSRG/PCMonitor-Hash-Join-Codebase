#!/bin/bash
cd build

sudo perf stat -e cpu-clock sudo ./Pool-HashJoin-PCM --id=0 --r-size=2000000000 --s-size=2000000000 --total-cores=50 --task-size=1000000 --skew=0 --core-pausing=0 --program-pmu=1 --hj-threads=1

#sudo perf stat -e cpu-clock sudo ./Pool-HashJoin-PCM --id=0 --r-size=2000000000  --s-size=2000000000 --total-cores=15 --task-size=1000000 --skew=0 --core-pausing=0 --program-pmu=1

# MEMORY SIZE EXPERIMENTS
# 32 GB -- previous default max
#sudo perf stat -e cpu-clock sudo ./Pool-HashJoin-PCM --id=32 --r-size=2000000000  --s-size=2000000000 --total-cores=15 --task-size=1000000 --skew=0 --core-pausing=0 --program-pmu=1
## 40 GB
#sudo perf stat -e cpu-clock sudo ./Pool-HashJoin-PCM --id=40 --r-size=2500000000  --s-size=2500000000 --total-cores=15 --task-size=1000000 --skew=0 --core-pausing=0 --program-pmu=1
## 43 GB
#sudo perf stat -e cpu-clock sudo ./Pool-HashJoin-PCM --id=43 --r-size=2700000000  --s-size=2700000000 --total-cores=15 --task-size=1000000 --skew=0 --core-pausing=0 --program-pmu=1
## 46.5
#sudo perf stat -e cpu-clock sudo ./Pool-HashJoin-PCM --id=46 --r-size=2900000000  --s-size=2900000000 --total-cores=15 --task-size=1000000 --skew=0 --core-pausing=0 --program-pmu=1
## 48 GB
#sudo perf stat -e cpu-clock sudo ./Pool-HashJoin-PCM --id=48 --r-size=3000000000  --s-size=3000000000 --total-cores=15 --task-size=1000000 --skew=0 --core-pausing=0 --program-pmu=1
## 64 GB
#sudo perf stat -e cpu-clock sudo ./Pool-HashJoin-PCM --id=64 --r-size=4000000000  --s-size=4000000000 --total-cores=15 --task-size=1000000 --skew=0 --core-pausing=0 --program-pmu=1
## 96 GB
#sudo perf stat -e cpu-clock sudo ./Pool-HashJoin-PCM --id=1 --r-size=6000000000  --s-size=6000000000 --total-cores=15 --task-size=1000000 --skew=0 --core-pausing=0 --program-pmu=1
## 112 GB
#sudo perf stat -e cpu-clock sudo ./Pool-HashJoin-PCM --id=7 --r-size=7000000000  --s-size=7000000000 --total-cores=15 --task-size=1000000 --skew=0 --core-pausing=0 --program-pmu=1
## 128 GB
#sudo perf stat -e cpu-clock sudo ./Pool-HashJoin-PCM --id=8 --r-size=8000000000  --s-size=8000000000 --total-cores=15 --task-size=1000000 --skew=0 --core-pausing=0 --program-pmu=1
## 136 GB -- max possible

# SOCKET EXPERIMENTS
#sudo perf stat -e cpu-clock sudo ./Pool-HashJoin-PCM --id=1-8 --r-size=8500000000  --s-size=8500000000 --total-cores=4 --task-size=5000000 --skew=0 --core-pausing=0 --program-pmu=1


# NUMBER OF CORES EXPERIMENTS
#sudo perf stat -e cpu-clock sudo ./Pool-HashJoin-PCM --id=13 --r-size=8500000000  --s-size=8500000000 --total-cores=13 --task-size=5000000 --skew=0 --core-pausing=0 --program-pmu=1
#sudo perf stat -e cpu-clock sudo ./Pool-HashJoin-PCM --id=12 --r-size=8500000000  --s-size=8500000000 --total-cores=12 --task-size=5000000 --skew=0 --core-pausing=0 --program-pmu=1
#sudo perf stat -e cpu-clock sudo ./Pool-HashJoin-PCM --id=11 --r-size=8500000000  --s-size=8500000000 --total-cores=11 --task-size=5000000 --skew=0 --core-pausing=0 --program-pmu=1
#sudo perf stat -e cpu-clock sudo ./Pool-HashJoin-PCM --id=10 --r-size=8500000000  --s-size=8500000000 --total-cores=10 --task-size=5000000 --skew=0 --core-pausing=0 --program-pmu=1
#sudo perf stat -e cpu-clock sudo ./Pool-HashJoin-PCM --id=9 --r-size=8500000000  --s-size=8500000000 --total-cores=9 --task-size=5000000 --skew=0 --core-pausing=0 --program-pmu=1
#sudo perf stat -e cpu-clock sudo ./Pool-HashJoin-PCM --id=8 --r-size=8500000000  --s-size=8500000000 --total-cores=8 --task-size=5000000 --skew=0 --core-pausing=0 --program-pmu=1
#sudo perf stat -e cpu-clock sudo ./Pool-HashJoin-PCM --id=7 --r-size=8500000000  --s-size=8500000000 --total-cores=7 --task-size=5000000 --skew=0 --core-pausing=0 --program-pmu=1
#sudo perf stat -e cpu-clock sudo ./Pool-HashJoin-PCM --id=6 --r-size=8500000000  --s-size=8500000000 --total-cores=6 --task-size=5000000 --skew=0 --core-pausing=0 --program-pmu=1
#sudo perf stat -e cpu-clock sudo ./Pool-HashJoin-PCM --id=5 --r-size=8500000000  --s-size=8500000000 --total-cores=5 --task-size=5000000 --skew=0 --core-pausing=0 --program-pmu=1
#sudo perf stat -e cpu-clock sudo ./Pool-HashJoin-PCM --id=4 --r-size=8500000000  --s-size=8500000000 --total-cores=4 --task-size=5000000 --skew=0 --core-pausing=0 --program-pmu=1
#sudo perf stat -e cpu-clock sudo ./Pool-HashJoin-PCM --id=3 --r-size=8500000000  --s-size=8500000000 --total-cores=3 --task-size=5000000 --skew=0 --core-pausing=0 --program-pmu=1
#sudo perf stat -e cpu-clock sudo ./Pool-HashJoin-PCM --id=2 --r-size=8500000000  --s-size=8500000000 --total-cores=2 --task-size=5000000 --skew=0 --core-pausing=0 --program-pmu=1
#sudo perf stat -e cpu-clock sudo ./Pool-HashJoin-PCM --id=1 --r-size=8500000000  --s-size=8500000000 --total-cores=1 --task-size=5000000 --skew=0 --core-pausing=0 --program-pmu=1


# CONCURRENT EXPERIMENTS
#(sudo perf stat -e cpu-clock sudo ./Pool-HashJoin-PCM --id=0 --r-size=2000000000  --s-size=2000000000 --total-cores=15 --task-size=1000000 --skew=0 --core-pausing=0 --program-pmu=1) &
#(
###sleep 4
#sudo perf stat -e cpu-clock sudo ./Pool-HashJoin-PCM --id=1 --r-size=2000000000  --s-size=2000000000 --total-cores=15 --task-size=1000000 --skew=0 --core-pausing=1 --program-pmu=1) &
#(
###sleep 8
#sudo perf stat -e cpu-clock sudo ./Pool-HashJoin-PCM --id=2 --r-size=2000000000  --s-size=2000000000 --total-cores=15 --task-size=1000000 --skew=0 --core-pausing=1 --program-pmu=1) &
#(
###sleep 12
#sudo perf stat -e cpu-clock sudo ./Pool-HashJoin-PCM --id=3 --r-size=2000000000  --s-size=2000000000 --total-cores=15 --task-size=1000000 --skew=0 --core-pausing=1 --program-pmu=1) &
##(
##sleep 45
##sudo perf stat -e cpu-clock sudo ./Pool-HashJoin-PCM --id=4 --r-size=2000000000  --s-size=2000000000 --total-cores=15 --task-size=1000000 --skew=0 --core-pausing=0 --program-pmu=1) &


# LOOP EXPERIMENTS
#for i in {1..10}
  #do
  #  cd build
  #  sudo ./Pool-HashJoin-PCM --r-size=2000000000 --s-size=2000000000 --total-cores=14 --task-size=1000000 --skew=0
  #
  #  cd ..
  #  ./plot.sh
#done
