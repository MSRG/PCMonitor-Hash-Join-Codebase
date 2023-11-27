#!/bin/bash
cd build

# Arguments:
# --id: unique id.
# --r-size: size of relation R (the one that is stored as hash table).
# --s-size: the size of relation S (the one that is probed).
# --total-cores: total number of cores to USE for the hash join.
# --task-size: size of build and probe tasks.
# --hj-threads: number of threads to be used for thread-based hash join. If 0, only one main thread uses for the process.
# --share-hashtable: [bool] if hash table sharing should be enabled.
# --monitor-cores: how many cores to monitor (regardless of how many are being used).
# --skew: type of skew to use for the data generation (0 = no skew).
# --core-pausing: [bool] is core pausing enabled.
# --program-pmu: [bool] should PMU be programmed and used (i.e. monitoring).

sudo perf stat -o ../cpu-cycles/cpu-cycles-0.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=0 --r-size=20000000 --s-size=20000000 --total-cores=4 --monitor-cores=4 --task-size=1000000 --hj-threads=1 --share-hash-table=0 --skew=0 --core-pausing=0 --program-pmu=1

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

#sudo perf stat -e cpu-clock

#  (sudo ./Pool-HashJoin-PCM --id=0 --r-size=1000 --s-size=2000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --skew=0 --core-pausing=0 --program-pmu=1) &
#  (
#  sudo ./Pool-HashJoin-PCM --id=1 --r-size=1000 --s-size=2000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --skew=0 --core-pausing=0 --program-pmu=1) &
#  (
#  sudo ./Pool-HashJoin-PCM --id=2 --r-size=1000 --s-size=2000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --skew=0 --core-pausing=0 --program-pmu=1) &
#  (
#  sudo ./Pool-HashJoin-PCM --id=3 --r-size=1000 --s-size=2000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --skew=0 --core-pausing=0 --program-pmu=1) &
#  (
#  sudo ./Pool-HashJoin-PCM --id=4 --r-size=1000 --s-size=2000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --skew=0 --core-pausing=0 --program-pmu=1) &
#  (
#  sudo ./Pool-HashJoin-PCM --id=5 --r-size=1000 --s-size=2000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --skew=0 --core-pausing=0 --program-pmu=1) &
#  (
#  sudo ./Pool-HashJoin-PCM --id=6 --r-size=1000 --s-size=2000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --skew=0 --core-pausing=0 --program-pmu=1) &
#  (
#  sudo ./Pool-HashJoin-PCM --id=7 --r-size=1000 --s-size=2000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --skew=0 --core-pausing=0 --program-pmu=1) &
#  (
#  sudo ./Pool-HashJoin-PCM --id=8 --r-size=1000 --s-size=2000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --skew=0 --core-pausing=0 --program-pmu=1) &
#  (
#  sudo ./Pool-HashJoin-PCM --id=9 --r-size=1000 --s-size=2000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --skew=0 --core-pausing=0 --program-pmu=1) &
#  (
#  sudo ./Pool-HashJoin-PCM --id=10 --r-size=1000 --s-size=2000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --skew=0 --core-pausing=0 --program-pmu=1) &
#  (
#  sudo ./Pool-HashJoin-PCM --id=11 --r-size=1000 --s-size=2000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --skew=0 --core-pausing=0 --program-pmu=1) &
#  (
#  sudo ./Pool-HashJoin-PCM --id=12 --r-size=1000 --s-size=2000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --skew=0 --core-pausing=0 --program-pmu=1) &
#  (
#  sudo ./Pool-HashJoin-PCM --id=13 --r-size=1000 --s-size=2000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --skew=0 --core-pausing=0 --program-pmu=1) &
#  (
#  sudo ./Pool-HashJoin-PCM --id=14 --r-size=1000 --s-size=2000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --skew=0 --core-pausing=0 --program-pmu=1) &
#  (
#  sudo ./Pool-HashJoin-PCM --id=15 --r-size=1000 --s-size=2000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --skew=0 --core-pausing=0 --program-pmu=1) &
#  (
#  sudo ./Pool-HashJoin-PCM --id=16 --r-size=1000 --s-size=2000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --skew=0 --core-pausing=0 --program-pmu=1) &

## CONCURRENT EXPERIMENTS
#start_time=$(date +%s.%N)

#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-0.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=0 --r-size=1000 --s-size=2000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=0 --program-pmu=1) &
#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-1.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=1 --r-size=1000 --s-size=2000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=0 --program-pmu=1) &
#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-2.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=2 --r-size=1000 --s-size=2000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=0 --program-pmu=1) &
#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-3.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=3 --r-size=1000 --s-size=2000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=0 --program-pmu=1) &
#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-4.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=4 --r-size=1000 --s-size=2000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=0 --program-pmu=1) &
#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-5.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=5 --r-size=1000 --s-size=2000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=0 --program-pmu=1) &
#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-6.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=6 --r-size=1000 --s-size=2000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=0 --program-pmu=1) &
#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-7.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=7 --r-size=1000 --s-size=2000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=0 --program-pmu=1) &
#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-8.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=8 --r-size=1000 --s-size=2000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=0 --program-pmu=1) &
#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-9.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=9 --r-size=1000 --s-size=2000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=1 --program-pmu=1) &
#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-10.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=10 --r-size=1000 --s-size=2000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=1 --program-pmu=1) &
#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-11.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=11 --r-size=1000 --s-size=2000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=1 --program-pmu=1) &
#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-12.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=12 --r-size=1000 --s-size=2000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=1 --program-pmu=1) &
#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-13.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=13 --r-size=1000 --s-size=2000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=1 --program-pmu=1) &
#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-14.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=14 --r-size=1000 --s-size=2000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=1 --program-pmu=1) &
#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-15.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=15 --r-size=1000 --s-size=2000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=1 --program-pmu=1) &
#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-16.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=16 --r-size=1000 --s-size=2000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=1 --program-pmu=1) &

# -- 17
#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-17.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=17 --r-size=1000 --s-size=2000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=0 --program-pmu=1) &
#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-18.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=18 --r-size=1000 --s-size=2000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=0 --program-pmu=1) &
#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-19.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=19 --r-size=1000 --s-size=2000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=0 --program-pmu=1) &
#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-20.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=20 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=1 --program-pmu=1) &
#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-21.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=21 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=1 --program-pmu=1) &
#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-22.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=22 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=1 --program-pmu=1) &
#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-23.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=23 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=1 --program-pmu=1) &
#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-24.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=24 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=1 --program-pmu=1) &
#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-25.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=25 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=1 --program-pmu=1) &
#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-26.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=26 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=1 --program-pmu=1) &
#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-27.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=27 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=1 --program-pmu=1) &
#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-28.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=28 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=1 --program-pmu=1) &
#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-29.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=29 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=1 --program-pmu=1) &
#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-30.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=30 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=1 --program-pmu=1) &
#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-31.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=31 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=1 --program-pmu=1) &
#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-32.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=32 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=1 --program-pmu=1) &
#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-33.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=33 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=1 --program-pmu=1) &
#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-34.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=34 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=1 --program-pmu=1) &
#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-35.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=35 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=1 --program-pmu=1) &
#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-36.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=36 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=1 --program-pmu=1) &
#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-37.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=37 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=0 --program-pmu=1) &
#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-38.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=38 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=0 --program-pmu=1) &
#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-39.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=39 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=0 --program-pmu=1) &
#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-40.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=40 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=0 --program-pmu=1) &
#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-41.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=41 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=0 --program-pmu=1) &

#wait
#end_time=$(date +%s.%N)
#
#execution_time=$(echo "$end_time - $start_time" | bc)
#echo "Execution time: $execution_time seconds"



### CONCURRENT EXPERIMENTS
#start_time=$(date +%s.%N)
#
#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-0.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=0 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=1 --program-pmu=1) &
#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-1.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=1 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=1 --program-pmu=1) &
#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-2.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=2 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=1 --program-pmu=1) &
#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-3.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=3 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=1 --program-pmu=1) &
#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-4.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=4 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=1 --program-pmu=1) &
#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-5.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=5 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=1 --program-pmu=1) &
#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-6.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=6 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=1 --program-pmu=1) &
#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-7.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=7 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=1 --program-pmu=1) &
#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-8.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=8 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=1 --program-pmu=1) &
#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-9.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=9 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=1 --program-pmu=1) &
#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-10.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=10 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=1 --program-pmu=1) &
#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-11.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=11 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=1 --program-pmu=1) &
#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-12.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=12 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=1 --program-pmu=1) &
#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-13.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=13 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=1 --program-pmu=1) &
#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-14.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=14 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=1 --program-pmu=1) &
#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-15.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=15 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=1 --program-pmu=1) &
#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-16.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=16 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=1 --program-pmu=1) &
#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-17.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=17 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=1 --program-pmu=1) &
#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-18.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=18 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=1 --program-pmu=1) &
#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-19.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=19 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=1 --program-pmu=1) &
#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-20.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=20 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=1 --program-pmu=1) &
#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-21.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=21 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=1 --program-pmu=1) &
#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-22.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=22 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=1 --program-pmu=1) &
#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-23.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=23 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=1 --program-pmu=1) &
#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-24.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=24 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=1 --program-pmu=1) &
#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-25.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=25 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=1 --program-pmu=1) &
#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-26.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=26 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=1 --program-pmu=1) &
#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-27.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=27 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=1 --program-pmu=1) &
#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-28.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=28 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=1 --program-pmu=1) &
#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-29.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=29 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=1 --program-pmu=1) &
#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-30.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=30 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=1 --program-pmu=1) &
#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-31.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=31 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=1 --program-pmu=1) &
#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-32.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=32 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=1 --program-pmu=1) &
#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-33.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=33 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=1 --program-pmu=1) &
#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-34.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=34 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=1 --program-pmu=1) &
#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-35.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=35 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=1 --program-pmu=1) &
#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-36.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=36 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=1 --program-pmu=1) &
#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-37.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=37 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=0 --program-pmu=1) &
#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-38.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=38 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=0 --program-pmu=1) &
#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-39.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=39 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=0 --program-pmu=1) &
#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-40.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=40 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=0 --program-pmu=1) &
#  (
#  sudo perf stat -o ../cpu-cycles/cpu-cycles-41.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=41 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=0 --program-pmu=1) &
##  (
##  sudo perf stat -o ../cpu-cycles/cpu-cycles-42.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=42 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=0 --program-pmu=1) &
#
#wait
#end_time=$(date +%s.%N)
#
#execution_time=$(echo "$end_time - $start_time" | bc)
#echo "Execution time: $execution_time seconds"


## SEQUENTIAL EXPERIMENTS
#start_time=$(date +%s.%N)
#
#sudo perf stat -o ../cpu-cycles/cpu-cycles-0.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=0 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=0 --program-pmu=1
#wait
#sudo perf stat -o ../cpu-cycles/cpu-cycles-1.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=1 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=0 --program-pmu=1
#wait
#sudo perf stat -o ../cpu-cycles/cpu-cycles-2.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=2 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=0 --program-pmu=1
#wait
#sudo perf stat -o ../cpu-cycles/cpu-cycles-3.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=3 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=0 --program-pmu=1
#wait
#sudo perf stat -o ../cpu-cycles/cpu-cycles-4.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=4 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=0 --program-pmu=1
#wait
#sudo perf stat -o ../cpu-cycles/cpu-cycles-5.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=5 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=0 --program-pmu=1
#wait
#sudo perf stat -o ../cpu-cycles/cpu-cycles-6.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=6 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=0 --program-pmu=1
#wait
#sudo perf stat -o ../cpu-cycles/cpu-cycles-7.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=7 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=0 --program-pmu=1
#wait
#sudo perf stat -o ../cpu-cycles/cpu-cycles-8.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=8 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=0 --program-pmu=1
#wait
#sudo perf stat -o ../cpu-cycles/cpu-cycles-9.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=9 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=0 --program-pmu=1
#wait
#sudo perf stat -o ../cpu-cycles/cpu-cycles-10.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=10 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=0 --program-pmu=1
#wait
#sudo perf stat -o ../cpu-cycles/cpu-cycles-11.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=11 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=0 --program-pmu=1
#wait
#sudo perf stat -o ../cpu-cycles/cpu-cycles-12.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=12 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=0 --program-pmu=1
#wait
#sudo perf stat -o ../cpu-cycles/cpu-cycles-13.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=13 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=0 --program-pmu=1
#wait
#sudo perf stat -o ../cpu-cycles/cpu-cycles-14.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=14 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=0 --program-pmu=1
#wait
#sudo perf stat -o ../cpu-cycles/cpu-cycles-15.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=15 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=0 --program-pmu=1
#wait
#sudo perf stat -o ../cpu-cycles/cpu-cycles-16.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=16 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=0 --program-pmu=1
#wait
#sudo perf stat -o ../cpu-cycles/cpu-cycles-17.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=17 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=0 --program-pmu=1
#wait
#sudo perf stat -o ../cpu-cycles/cpu-cycles-18.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=18 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=0 --program-pmu=1
#wait
#sudo perf stat -o ../cpu-cycles/cpu-cycles-19.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=19 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=0 --program-pmu=1
#wait
#sudo perf stat -o ../cpu-cycles/cpu-cycles-20.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=20 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=0 --program-pmu=1
#wait
#sudo perf stat -o ../cpu-cycles/cpu-cycles-21.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=21 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=0 --program-pmu=1
#wait
#sudo perf stat -o ../cpu-cycles/cpu-cycles-22.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=22 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=0 --program-pmu=1
#wait
#sudo perf stat -o ../cpu-cycles/cpu-cycles-23.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=23 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=0 --program-pmu=1
#wait
#sudo perf stat -o ../cpu-cycles/cpu-cycles-24.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=24 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=0 --program-pmu=1
#wait
#sudo perf stat -o ../cpu-cycles/cpu-cycles-25.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=25 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=0 --program-pmu=1
#wait
#sudo perf stat -o ../cpu-cycles/cpu-cycles-26.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=26 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=0 --program-pmu=1
#wait
#sudo perf stat -o ../cpu-cycles/cpu-cycles-27.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=27 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=0 --program-pmu=1
#wait
#sudo perf stat -o ../cpu-cycles/cpu-cycles-28.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=28 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=0 --program-pmu=1
#wait
#sudo perf stat -o ../cpu-cycles/cpu-cycles-29.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=29 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=0 --program-pmu=1
#wait
#sudo perf stat -o ../cpu-cycles/cpu-cycles-30.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=30 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=0 --program-pmu=1
#wait
#sudo perf stat -o ../cpu-cycles/cpu-cycles-31.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=31 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=0 --program-pmu=1
#wait
#sudo perf stat -o ../cpu-cycles/cpu-cycles-32.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=32 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=0 --program-pmu=1
#wait
#sudo perf stat -o ../cpu-cycles/cpu-cycles-33.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=33 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=0 --program-pmu=1
#wait
#sudo perf stat -o ../cpu-cycles/cpu-cycles-34.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=34 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=0 --program-pmu=1
#wait
#sudo perf stat -o ../cpu-cycles/cpu-cycles-35.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=35 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=0 --program-pmu=1
#wait
#sudo perf stat -o ../cpu-cycles/cpu-cycles-36.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=36 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=0 --program-pmu=1
#wait
#sudo perf stat -o ../cpu-cycles/cpu-cycles-37.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=37 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=0 --program-pmu=1
#wait
#sudo perf stat -o ../cpu-cycles/cpu-cycles-38.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=38 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=0 --program-pmu=1
#wait
#sudo perf stat -o ../cpu-cycles/cpu-cycles-39.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=39 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=0 --program-pmu=1
#wait
#sudo perf stat -o ../cpu-cycles/cpu-cycles-40.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=40 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=0 --program-pmu=1
#wait
#sudo perf stat -o ../cpu-cycles/cpu-cycles-41.txt -e cpu-cycles sudo ./Pool-HashJoin-PCM --id=41 --r-size=1000 --s-size=1000000000 --total-cores=4 --monitor-cores=15 --task-size=1000000 --hj-threads=0 --share-hash-table=0 --skew=0 --core-pausing=0 --program-pmu=1
#wait
#
#end_time=$(date +%s.%N)
#execution_time=$(echo "$end_time - $start_time" | bc)
#echo "Execution time: $execution_time seconds"



#  (
        #  sleep 12
#  sudo ./Pool-HashJoin-PCM --id=8 --r-size=1000000000 --s-size=1000000000 --total-cores=4 --task-size=1000000 --skew=0 --core-pausing=0 --program-pmu=1) &
#  (
        #  sleep 12
#  sudo ./Pool-HashJoin-PCM --id=9 --r-size=1000000000 --s-size=1000000000 --total-cores=15 --task-size=1000000 --skew=0 --core-pausing=0 --program-pmu=1) &
#  (
    #  sleep 12
#  sudo ./Pool-HashJoin-PCM --id=4 --r-size=100000000 --s-size=100000000 --total-cores=15 --task-size=1000000 --skew=0 --core-pausing=0 --program-pmu=1) &
#  (
#  sleep 45
#  sudo perf stat -e cpu-clock sudo ./Pool-HashJoin-PCM --id=4 --r-size=2000000000  --s-size=2000000000 --total-cores=15 --task-size=1000000 --skew=0 --core-pausing=0 --program-pmu=1) &

#done

# LOOP EXPERIMENTS
#for i in {1..10}
  #do
  #  cd build
  #  sudo ./Pool-HashJoin-PCM --r-size=2000000000 --s-size=2000000000 --total-cores=14 --task-size=1000000 --skew=0
  #
  #  cd ..
  #  ./plot.sh
#done