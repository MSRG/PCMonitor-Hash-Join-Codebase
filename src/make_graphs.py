import matplotlib.pyplot as plt
import csv

png_file_name = '../plots/L3-Cache-Misses.png'

def getColumn(matrix, i):
    return [int(row[i]) for row in matrix]

def plotCache():

    with open('../build/cache-results.csv') as csvFile:
        rows = csv.reader(csvFile, delimiter=',')

        y = []
        all_rows = []

        for row in rows:
            all_rows.append(row)

        for i in range (14):
            y = getColumn(all_rows, i)
            x = list(range(0, len(y)))
            label = "core-"+str(i)
            plt.plot(x, y, linestyle = 'dashed', marker = 'o',label = label)

    plt.xticks(rotation = 25)
    plt.xlabel('Checkpoint')
    plt.ylabel('L3 Cache Misses')
    plt.title('L3 Cache Misses per Core', fontsize = 20)
    plt.grid()
    plt.legend(loc=(1.04, 0))
    plt.savefig(png_file_name, bbox_inches='tight')

if __name__ == "__main__":
    plotCache()
    print("Saved L3 Cache Misses Plot to " + png_file_name + ".")