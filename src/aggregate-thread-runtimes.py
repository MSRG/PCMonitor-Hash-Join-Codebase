import os
import shutil

def copy_time(src_file, dest_file):
    with open(src_file, 'r') as src, open(dest_file, 'a') as dest:
        line = src.readline().strip()
        dest.write(line + '\n')

def parse_and_copy(directory_path, output_file):
    with open(output_file, 'w') as output:
        output.write('')  # Create an empty output file or clear existing content
    for root, dirs, files in os.walk(directory_path):
        for file in files:
            file_path = os.path.join(root, file)
            copy_time(file_path, output_file)

if __name__ == "__main__":
    source_directory = "../individual-thread-timing"  # Change this to the path of your source directory
    output_file_path = '../results-consolidated/consolidated-thread-runtimes.csv'  # Change this to the desired output file path

    parse_and_copy(source_directory, output_file_path)
    print(f"Contents of files in '{source_directory}' have been copied to '{output_file_path}'.")