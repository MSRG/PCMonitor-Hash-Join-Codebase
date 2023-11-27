#!/bin/bash

sudo chmod -R 777 results/
sudo chmod -R 777 cpu-cycles/

cd results/
sudo rm -r *

cd ../cpu-cycles/
sudo rm -r *