#!/bin/bash

sudo chmod -R 777 results/
cd src
python make_graphs.py
cd ..
#./push-plots.sh
./push-results.sh