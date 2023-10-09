#!/bin/bash

# save graphs
cd src
python save_results.py

# save
cd ..
./push-plots.sh