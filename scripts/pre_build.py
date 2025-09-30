import os
import shutil
from pathlib import Path

# This script copies the data directory to the build directory
# so that it can be uploaded to the ESP32's filesystem

def copy_data_dir(source, destination):
    if not os.path.exists(destination):
        os.makedirs(destination)
    
    for item in os.listdir(source):
        s = os.path.join(source, item)
        d = os.path.join(destination, item)
        
        if os.path.isdir(s):
            copy_data_dir(s, d)
        else:
            shutil.copy2(s, d)

# Get the project directory
project_dir = Path("__file__").parent.parent
data_dir = os.path.join(project_dir, "data")
dest_dir = os.path.join(project_dir, ".pio", "build", "esp32devV3x", "data")

# Copy data directory to build directory
if os.path.exists(data_dir):
    if not os.path.exists(dest_dir):
        os.makedirs(dest_dir)
    copy_data_dir(data_dir, dest_dir)
    print("Data directory copied to build directory")
else:
    print("Data directory not found")
