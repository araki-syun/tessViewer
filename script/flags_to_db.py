import json
import pathlib
import argparse

parser = argparse.ArgumentParser(
    epilog="Example : python flags_to_db.py foo/compile_flags.txt")
parser.add_argument("file",
                    type=argparse.FileType("r"),
                    help="compile option file")
args = parser.parse_args()

options = ""
f = object()
if args.file == None:
    f = open("compile_flags.txt", 'r', encoding="utf-8")
else:
    f = args.file
for option in f:
    options += "\"" + option.replace("\n", "") + "\" "
f.close()

options += " -c -o"

current = pathlib.Path.cwd()
print(f"project root : {current}")
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
