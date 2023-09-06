#!/bin/bash

cd src
python make_graphs.py
cd ..
./push-plots.sh