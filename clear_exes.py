import os

def delete_executables(root_dir='.'):
    deleted_files = []
    for dirpath, dirnames, filenames in os.walk(root_dir):
        for filename in filenames:
            if filename.lower().endswith('.exe'):
                file_path = os.path.join(dirpath, filename)
                try:
                    os.remove(file_path)
                    deleted_files.append(file_path)
                    print(f"Deleted: {file_path}")
                except Exception as e:
                    print(f"Failed to delete {file_path}: {e}")
    if not deleted_files:
        print("No executable files found.")
    else:
        print(f"\nTotal deleted: {len(deleted_files)}")

if __name__ == "__main__":
    delete_executables()
