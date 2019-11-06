import os

directory = os.path.dirname(__file__)
schema = directory + r"/setting_schema.json"
out = schema + r".gen.h"

with open(out, "w", encoding="utf-8") as h:
#    h.write(r"#pragma once" + "\n")
#    h.write(r"namespace glapp::schema{")
    h.write(r"constexpr char setting_json[] =" + "\n")
    with open(schema, "r", encoding="utf-8") as f:
        for line in f:
            s = line.replace(r'"', r'\"').replace("\n","")
            h.write(f'\"{s}\"\n')
    h.write(r";")
#    h.write(r"} // namespace glapp::schema")
