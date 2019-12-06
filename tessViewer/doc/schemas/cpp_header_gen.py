import os
import sys
import json
import argparse

parser = argparse.ArgumentParser()
parser.add_argument("--indent",
                    type=int,
                    default=0,
                    help="indent width")
args = parser.parse_args()

directory = os.path.dirname(__file__)
schema = directory + r"/setting_schema.json"
out = schema + r".gen.h"

with open(out, "w", encoding="utf-8") as h:
    h.write("constexpr char setting_json[] =\nR\"(")
    with open(schema, "r", encoding="utf-8") as f:
        j = json.load(f)
        if args.indent == 0:
            h.write(json.dumps(j))
        else:
            h.write(json.dumps(j, indent=args.indent))
    h.write(")\";")
sys.exit()
