import os

def reorder_folders(source_path):
    # Get a list of directories in the source path
    folders = [f for f in os.listdir(source_path) if os.path.isdir(os.path.join(source_path, f))]

    # Reorder the folders based on the last two digits of their names
    folders.sort(key=lambda x: int(x[-2:]))

    # Create a new order for the folders
    new_order = [os.path.join(source_path, folder) for folder in folders]

    # Move the folders to their new locations
    for old_path, new_path in zip(folders, new_order):
        os.rename(old_path, new_path)

if __name__ == "__main__":
    # Specify the path to the folder containing the subfolders to be reordered
    source_folder_path = "../results-copy/"

    reorder_folders(source_folder_path)