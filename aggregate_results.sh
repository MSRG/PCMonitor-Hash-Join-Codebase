#!/bin/bash

sudo chmod -R 777 results/
cd src
python aggregate_runtimes.py
python aggregate_cpu_cycles.py
python aggregate_thread_runtimes.py