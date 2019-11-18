import json
import os
import pathlib

options = ""

with open("compile_flags.txt", 'r', encoding="utf-8") as flags:
    for option in flags:
        options += option.replace("\n", " ")
options += " -c -o"

current = pathlib.Path.cwd()
print(current)
db = []
for file in current.rglob(r"*"):
    if pathlib.re.search(r"^.*\.(h|hpp|c|cpp)$", file.name):
        file_path = str(file.relative_to(current)).replace("\\", "/")
        print(file_path)
        db.append({"directory": str(current).replace("\\", "/"),
                   "command": f"clang++.exe {options} tessViewer/x64/{file.name}.o {file_path}",
                   "file": file_path})

with open("compile_commands.json", 'w', encoding="utf-8") as compile_commands:
    json.dump(db, compile_commands, indent=4)
