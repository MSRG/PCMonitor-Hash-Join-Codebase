# This script was made to copy result files from /build folder into it's
# own unique folder in /results.
# Since writing this script, the code has been updated to automatically do this.
# That allows for multiple hash joins to run concurrently.

import csv
import os
import shutil
from datetime import datetime

current_time    = datetime.now()
current_time    = current_time.strftime("%m-%d-%Y-%H:%M:%S")
folder_path     = '../results/'+current_time

timing_file_name    = '../build/timing-results.csv'
tresults_file_name  = '../build/individual-thread-results.csv'
mb_file_name        = '../build/MB-results.csv'
ipc_file_name       = '../build/IPC-results.csv'
cache_file_name     = '../build/cache-results.csv'

timing_file_path    = folder_path+'/timing-results.csv'
tresults_file_path  = folder_path+'/individual-thread-results.csv'
mb_file_path        = folder_path+'/MB-results.csv'
ipc_file_path       = folder_path+'/IPC-results.csv'
cache_file_path     = folder_path+'/cache-results.csv'

def saveFiles():

    shutil.copyfile(timing_file_name, timing_file_path)
    shutil.copyfile(tresults_file_name, tresults_file_path)
    shutil.copyfile(mb_file_name, mb_file_path)
    shutil.copyfile(ipc_file_name, ipc_file_path)
    shutil.copyfile(cache_file_name, cache_file_path)

if __name__ == "__main__":

    if not os.path.exists(folder_path):
        os.makedirs(folder_path)
        print(f"Created folder '{folder_path}'.")
    else:
        print(f"Folder '{folder_path}' already exists.")

    saveFiles()