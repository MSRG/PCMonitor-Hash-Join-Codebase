#!/bin/bash

sudo chmod -R 777 results/
cd src
python aggregate-runtimes.py
python aggregate-cpu-cycles.py
