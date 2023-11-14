import os
import shutil

# def copy_csv_contents(src_file, dest_file):
#     with open(src_file, 'r') as src, open(dest_file, 'a') as dest:
#         dest.write(src.read())

def copy_second_line(src_file, dest_file):
    with open(src_file, 'r') as src, open(dest_file, 'a') as dest:
        second_line = src.readline().strip()
        second_line = src.readline().strip()
        dest.write(second_line + '\n')

def parse_and_copy(directory_path, output_file):
    with open(output_file, 'w') as output:
        output.write('')  # Create an empty output file or clear existing content
    for root, dirs, files in os.walk(directory_path):
        if 'individual-thread-results.csv' in files:
            file_path = os.path.join(root, 'timing-results.csv')
            copy_second_line(file_path, output_file)

if __name__ == "__main__":
    source_directory = "../results"  # Change this to the path of your source directory
    output_file_path = '../results-consolidated/consolidated-results.csv'  # Change this to the desired output file path

    parse_and_copy(source_directory, output_file_path)
    print(f"Contents of 'timing-results.csv' files in '{source_directory}' have been copied to '{output_file_path}'.")
