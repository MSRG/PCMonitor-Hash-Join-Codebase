import matplotlib.pyplot as plt
import pandas as pd
import csv
import os
from datetime import datetime

# current_time = datetime.now()
# current_time = current_time.strftime("%m-%d-%Y-%H:%M:%S")
# folder_path = '../plots/'+current_time
# cache_png_file_name = folder_path+'/L2-Cache-Misses.png'
# ipc_png_file_name = folder_path+'/IPC.png'
# mb_png_file_name = folder_path+'/RM.png'
# lmb_png_file_name = folder_path+'/local-RM.png'
# rmb_png_file_name = folder_path+'/remote-RM.png'
# tresults_png_file_name = folder_path+'/Individual-Thread-Results.png'

# numThreads = 15

cache_csv_file_name = '/cache-results.csv'
ipc_csv_file_name = '/IPC-results.csv'
mb_csv_file_name = '/MB-results.csv'
tresults_csv_file_name = '/individual-thread-results.csv'
individual_thread_timing_csv_file_name = '../results-consolidated/consolidated-thread-runtimes.csv'

cache_png_file_name = '/cache-results.png'
ipc_png_file_name = '/IPC-results.png'
mb_png_file_name = '/MB-results.png'
lmb_png_file_name = '/MB-local-results.png'
rmb_png_file_name = '/MB-remote-results.png'
tresults_png_file_name = '/individual-thread-results.png'
individual_thread_timing_png_file_name = '/individual-thread-timing-results.png'

nThreads = 0

def getColumnInt(matrix, i):
    return [int(row[i]) for row in matrix]

def getColumnDob(matrix, i):
    return [float(row[i]) for row in matrix]

def plotCache(path):
    plt.clf()
    y = []
    all_rows = []

    print(path + cache_csv_file_name)
    with open(path + cache_csv_file_name) as csvFile:
        rows = csv.reader(csvFile, delimiter=',')

        # Find number of threads in the file:
        nThreads = len(next(rows)) # Read first line and count columns
        csvFile.seek(0)

        for row in rows:
            all_rows.append(row)

        for i in range (nThreads):
            y = getColumnInt(all_rows, i)
            x = list(range(0, len(y)))
            label = "core-"+str(i)
            plt.plot(x, y, linestyle = 'dashed', marker = 'o',label = label)

        plt.xticks(rotation = 25)
        plt.xlabel('Checkpoint')
        plt.ylabel('L2 Cache Misses')
        plt.title('L2 Cache Misses per Core', fontsize = 20)
        plt.grid()
        plt.legend(loc=(1.04, 0))
        plt.savefig(path + "/plots" + cache_png_file_name, bbox_inches='tight')


def plotIpc(path):
    plt.clf()
    y = []
    all_rows = []

    with open(path + ipc_csv_file_name) as csvFile:
        rows = csv.reader(csvFile, delimiter=',')

#         Find number of threads in the file:
        nThreads = len(next(rows)) # Read first line and count columns
        csvFile.seek(0)

        for row in rows:
            all_rows.append(row)

        for i in range (nThreads):
            y = getColumnDob(all_rows, i)
            x = list(range(0, len(y)))
            label = "core-"+str(i)
            plt.plot(x, y, linestyle = 'dashed', marker = 'o',label = label)

        plt.xticks(rotation = 25)
        plt.xlabel('Checkpoint')
        plt.ylabel('IPC')
        plt.title('IPC per Core')
        plt.grid()
        plt.legend(loc=(1.04, 0))
        plt.savefig(path + "/plots" + ipc_png_file_name, bbox_inches='tight')


# Individual thread timing results
def plotThreadTimingResults(path):
    plt.clf()

    # Read the CSV file into a DataFrame
    df = pd.read_csv(individual_thread_timing_csv_file_name, header=None, names=['runtimes'])

    # Plotting
    plt.bar(range(len(df)), df['runtimes'])
    plt.xlabel('Thread ID')
    plt.ylabel('Runtime (s)')
    plt.title('Runtime of Individual Threads')
    plt.grid()
    plt.savefig(path + "/plots" + individual_thread_timing_png_file_name, bbox_inches='tight')


def plotThreadResultsSkew(path):
    plt.clf()
    data = []
    x_2 = []
    x_3 = []

    with open(path + tresults_csv_file_name) as csvFile:
        rows = csv.reader(csvFile, delimiter=',')

        for row in rows:
            data.append([int(row[0]), int(row[1]), int(row[2]), int(row[3]), int(row[4])])

        data.sort()
        x, tasks, matches, matchTasks, nonMatchTasks = zip(*data)

        processedTuples = tuple(x * 1000000 for x in tasks)

        plt.bar(x, processedTuples, color = 'b', width = 0.25, label = "Total Processed Tuples")

        for val in x:
            x_2.append(val+0.25)

        nonMatchTuples = tuple(processed - match for processed, match in zip(processedTuples, matches))

        plt.bar(x_2, matches, color = 'y', width = 0.25, label = "Matches")
        plt.bar(x_2, nonMatchTuples, bottom=matches, color = 'g', width = 0.25, label = "Non-Matches")

        for val in x:
            x_3.append(val+0.5)

        max_y = max(processedTuples)
        plt.ylim(0, max_y+8000000)

        plt.xlabel('Thread ID')
        plt.ylabel('Processed Tuples')
        plt.title('Tuples Processed per Thread')
        plt.legend(loc=(1.04, 0))
        plt.grid()
        plt.savefig(path + "/plots" + tresults_png_file_name, bbox_inches='tight')


def plotThreadResults(path):
    plt.clf()
    data = []
    x_2 = []
    x_3 = []

    with open(path + tresults_csv_file_name) as csvFile:
        rows = csv.reader(csvFile, delimiter=',')

        for row in rows:
            data.append([int(row[0]), int(row[1]), int(row[2]), int(row[3]), int(row[4])])

        data.sort()
        x, tasks, matches, matchTasks, nonMatchTasks = zip(*data)

        plt.bar(x, tasks, color = 'b', width = 0.25, label = "Total Completed Tasks")

        for val in x:
            x_2.append(val+0.25)

        plt.bar(x_2, matchTasks, color = 'y', width = 0.25, label = "Matches")
        plt.bar(x_2, nonMatchTasks, bottom=matchTasks, color = 'g', width = 0.25, label = "Non-Matches")

        for val in x:
            x_3.append(val+0.5)

        max_y = max(tasks)
        plt.ylim(0, max_y+20)

        plt.xlabel('Thread ID')
        plt.ylabel('Completed Tasks')
        plt.title('Completed Tasks per Thread')
        plt.legend(loc=(1.04, 0))
        plt.grid()
        plt.savefig(path + "/plots" + tresults_png_file_name, bbox_inches='tight')


def plotMemBandwidth(path):
    plt.clf()
    y = []
    all_rows = []

    with open(path + mb_csv_file_name) as csvFile:
        rows = csv.reader(csvFile, delimiter=',')

        for row in rows:
            all_rows.append(row)

        for i in range (plots):
            core = i
            for j in range (2):
                y = getColumnDob(all_rows, i)
                x = list(range(0, len(y)))
                if j == 1:
                    label = "local-core-"+str(core)
                else:
                    label = "remote-core-"+str(core)
                plt.plot(x, y, linestyle = 'dashed', marker = 'o', label = label)

        plt.xticks(rotation = 25)
        plt.xlabel('Checkpoint')
        plt.ylabel('IPC')
        plt.title('IPC per Core')
        plt.grid()
        plt.legend(loc=(1.04, 0))
        plt.savefig(path + "/plots" + mb_png_file_name, bbox_inches='tight')

def plotLocalMemBandwidth(path):
    plt.clf()
    y = []
    all_rows = []

    with open(path + mb_csv_file_name) as csvFile:
        rows = csv.reader(csvFile, delimiter=',')
        for row in rows:
            all_rows.append(row)

        core = 0
        for i in range (0,nThreads,2):

            y = getColumnDob(all_rows, i)
            x = list(range(0, len(y)))
            label = "local-core-"+str(core)
            plt.plot(x, y, linestyle = 'dashed', marker = 'o', label = label)
            core += 1

        plt.xticks(rotation = 25)
        plt.xlabel('Checkpoint')
        plt.ylabel('Unit')
        plt.title('Local Memory Bandwidth')
        plt.grid()
        plt.legend(loc=(1.04, 0))
        plt.savefig(path + "/plots" + lmb_png_file_name, bbox_inches='tight')

def plotRemoteMemBandwidth(path):
    plt.clf()
    y = []
    all_rows = []

    with open(path + mb_csv_file_name) as csvFile:
        rows = csv.reader(csvFile, delimiter=',')
        for row in rows:
            all_rows.append(row)

        core = 0

        for i in range (1,nThreads,2):
            y = getColumnDob(all_rows, i)
            x = list(range(0, len(y)))
            label = "remote-core-"+str(core)
            plt.plot(x, y, linestyle = 'dashed', marker = 'o', label = label)
            core += 1

        plt.xticks(rotation = 25)
        plt.xlabel('Checkpoint')
        plt.ylabel('Unit')
        plt.title('Remote Memory Bandwidth')
        plt.grid()
        plt.legend(loc=(1.04, 0))
        plt.savefig(path + "/plots" + rmb_png_file_name, bbox_inches='tight')


if __name__ == "__main__":

    results_path = "../results"
    dirNames = next(os.walk(results_path))[1]
    for directory in dirNames:

        directory = "../results/" + directory
        plot_dir = directory + "/plots"

        if not os.path.exists(plot_dir):
            os.makedirs(plot_dir)
            print(f"Created folder '{plot_dir}'.")
        else:
            print(f"Folder '{plot_dir}' already exists.")

        plotCache(directory)            # also finds the number of rows.
        plotIpc(directory)
        plotThreadResultsSkew(directory)
        plotThreadTimingResults(directory)
#             plotLocalMemBandwidth(directory)
#             plotRemoteMemBandwidth(directory)

#     plotCache()
#     plotIpc()
#     plotThreadResults()
#     plotLocalMemBandwidth()
#     plotRemoteMemBandwidth()

    print("Done plotting!\n")