import matplotlib.pyplot as plt
import csv
import os
from datetime import datetime

current_time = datetime.now()
current_time = current_time.strftime("%m-%d-%Y-%H:%M:%S")
folder_path = '../plots/'+current_time
cache_png_file_name = folder_path+'/L2-Cache-Misses.png'
ipc_png_file_name = folder_path+'/IPC.png'
mb_png_file_name = folder_path+'/RM.png'
lmb_png_file_name = folder_path+'/local-RM.png'
rmb_png_file_name = folder_path+'/remote-RM.png'

tresults_png_file_name = folder_path+'/Individual-Thread-Results.png'

def setFileNames():
    cache_png_file_name = '../plots/L2-Cache-Misses.png'
    ipc_png_file_name = '../plots/IPC.png'
    tresults_png_file_name = '../plots/Individual-Thread-Results.png'

def getColumnInt(matrix, i):
    return [int(row[i]) for row in matrix]

def getColumnDob(matrix, i):
    return [float(row[i]) for row in matrix]

def plotCache():
    y = []
    all_rows = []

    with open('../build/cache-results.csv') as csvFile:
        rows = csv.reader(csvFile, delimiter=',')

        for row in rows:
            all_rows.append(row)

        for i in range (14):
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
        plt.savefig(cache_png_file_name, bbox_inches='tight')


def plotIpc():
    plt.clf()
    y = []
    all_rows = []

    with open('../build/IPC-results.csv') as csvFile:
        rows = csv.reader(csvFile, delimiter=',')

        for row in rows:
            all_rows.append(row)

        for i in range (14):
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
        plt.savefig(ipc_png_file_name, bbox_inches='tight')


def plotThreadResults():
    plt.clf()
    data = []
    x_2 = []
    x_3 = []

    with open('../build/individual-thread-results.csv') as csvFile:
        rows = csv.reader(csvFile, delimiter=',')

        for row in rows:
            data.append([int(row[0]), int(row[1]), int(row[2]), int(row[3]), int(row[4])])

        data.sort()
        x, tasks, matches, matchTasks, nonMatchTasks = zip(*data)

        plt.bar(x, tasks, color = 'b', width = 0.25, label = "Total Completed Tasks")

        for val in x:
            x_2.append(val+0.25)

        plt.bar(x_2, matchTasks, color = 'y', width = 0.25, label = "Match Tasks")
        plt.bar(x_2, nonMatchTasks, bottom=matchTasks, color = 'g', width = 0.25, label = "Non Match Tasks")

        for val in x:
            x_3.append(val+0.5)

        plt.xlabel('Core ID')
        plt.ylabel('Completed Tasks')
        plt.title('Completed Tasks per Core')
        plt.legend(loc=(1.04, 0))
        plt.grid()
        plt.savefig(tresults_png_file_name, bbox_inches='tight')


def plotMemBandwidth():
    plt.clf()
    y = []
    all_rows = []

    with open('../build/MB-results.csv') as csvFile:
        rows = csv.reader(csvFile, delimiter=',')

        for row in rows:
            all_rows.append(row)

        for i in range (14):
            core = i
            for j in range (2):
                y = getColumnDob(all_rows, i)
                x = list(range(0, len(y)))
                if j == 1:
                    label = "local-core-"+str(core)
                else:
                    label = "remote-core-"+str(core)
                plt.plot(x, y, linestyle = 'dashed', marker = 'o', label = label)

#             y = getColumnDob(all_rows, i)
#             x = list(range(0, len(y)))
#             label = "core-"+str(i)
#             plt.plot(x, y, linestyle = 'dashed', marker = 'o', label = label)

        plt.xticks(rotation = 25)
        plt.xlabel('Checkpoint')
        plt.ylabel('IPC')
        plt.title('IPC per Core')
        plt.grid()
        plt.legend(loc=(1.04, 0))
        plt.savefig(mb_png_file_name, bbox_inches='tight')

def plotLocalMemBandwidth():
    plt.clf()
    y = []
    all_rows = []

    with open('../build/MB-results.csv') as csvFile:
        rows = csv.reader(csvFile, delimiter=',')
        for row in rows:
            all_rows.append(row)

        core = 0
        for i in range (0,28,2):

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
        plt.savefig(lmb_png_file_name, bbox_inches='tight')

def plotRemoteMemBandwidth():
    plt.clf()
    y = []
    all_rows = []

    with open('../build/MB-results.csv') as csvFile:
        rows = csv.reader(csvFile, delimiter=',')
        for row in rows:
            all_rows.append(row)

        core = 0

        for i in range (1,28,2):
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
        plt.savefig(rmb_png_file_name, bbox_inches='tight')


if __name__ == "__main__":

    if not os.path.exists(folder_path):
        os.makedirs(folder_path)
        print(f"Created folder '{folder_path}'.")
    else:
        print(f"Folder '{folder_path}' already exists.")

#     setFileNames()
    plotCache()
    plotIpc()
    plotThreadResults()
    plotLocalMemBandwidth()
    plotRemoteMemBandwidth()

    print("Done plotting!\n")