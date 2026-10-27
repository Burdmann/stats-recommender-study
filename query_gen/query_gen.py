import random
import sys
from datetime import datetime,date
from collections import defaultdict
import duckdb

print(sys.argv,file=sys.stderr)
table = sys.argv[1]
template = sys.argv[2]
count = int(sys.argv[3])
rest = sys.argv[4:]
columns,ids = rest[::2],rest[1::2]
ids = list(map(int,ids))

templatep = []
for x in template.split("?"):
    templatep.append(x)
    templatep.append("")
templatep.pop()
template = templatep

duckdb.execute(f"CREATE TABLE tbl AS SELECT * FROM '{table}';")

def gen(col):
    return duckdb.execute(f"SELECT \"{col}\" FROM (SELECT DISTINCT \"{col}\" FROM tbl) USING SAMPLE 1;").fetchone()[0]

def format_data(data):
    if isinstance(data,str):
        return f"'{data}'"
    if isinstance(data,datetime):
        fmt = "%Y-%m-%d %H:%M:%S"
        return f"'{datetime.strftime(data,fmt)}'"
    if isinstance(data,date):
        fmt = "%Y-%m-%d"
        return f"'{datetime.strftime(data,fmt)}'"
    if data is None:
        return "NULL"
    return str(data)

for _ in range(count):
    encounters = defaultdict(int)
    data = defaultdict(list)
    for i in range(len(columns)):
        template[1+2*i] = (ids[i],encounters[ids[i]])
        data[ids[i]].append(format_data(gen(columns[i])))
        encounters[ids[i]] += 1
    for key in data:
        data[key].sort()
    for i in range(len(columns)):
        j,k = template[1+2*i]
        template[1+2*i] = data[j][k]
    print("".join(template))