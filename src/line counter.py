import os
from pathlib import Path

def read_dir(dir) -> int:
    dir = Path(dir)
    linecount:int = 0
    for file_path in dir.rglob("*"):
        if file_path.is_file():
            with open(file_path, "r", encoding="utf-8") as file:
                for char in file.read():
                    if char == '\n': linecount += 1
    return linecount

core = read_dir('core')
etc  = read_dir('etc')
print(core)
print(etc)
print(core + etc)