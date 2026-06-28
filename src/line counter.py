import os
from pathlib import Path

def read_dir(dir) -> int:
    dir = Path(dir)
    linecount:int = 1
    for file_path in dir.rglob("*"):
        if file_path.is_file():
            local_line_count:int = 1
            with open(file_path, "r", encoding="utf-8") as file:
                for char in file.read():
                    if char == '\n': local_line_count += 1
            print(f"{file_path}: {local_line_count}")
            linecount += local_line_count
    return linecount

core = read_dir('core')
etc  = read_dir('etc')
print(core)
print(etc)
print(core + etc)