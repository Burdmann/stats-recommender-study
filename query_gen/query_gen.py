import random
import sys
from datetime import datetime,date
from collections import defaultdict
import duckdb

print(sys.argv,file=sys.stderr)
table = sys.argv[1]
template = sys.argv[2]
count = int(sys.argv[3])
profiler_dir = sys.argv[4]
rest = sys.argv[5:]
columns,ids,ps = rest[::3],rest[1::3],rest[2::3]
ids = list(map(int,ids))
ps = list(map(float,ps))

templatep = []
for x in template.split("?"):
    templatep.append(x)
    templatep.append("")
templatep.pop()
template = templatep

duckdb.execute("ATTACH 'test.db';")
duckdb.execute("USE test;")
# duckdb.execute(f"CREATE OR REPLACE TABLE tbl AS SELECT * FROM '{table}';")

def _gen(col):
    return duckdb.execute(f"SELECT \"{col}\" FROM (SELECT DISTINCT \"{col}\" FROM tbl) USING SAMPLE 1;").fetchone()[0]

def gen(col,p):
    if p == -1:
        return _gen(col)
    return duckdb.execute(f"SELECT quantile_cont({col}, {p}) FROM tbl;").fetchone()[0]

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
    id_count = defaultdict(int)
    data = defaultdict(list)
    start = {}
    for i in range(len(columns)):
        id_count[ids[i]] += 1
    for id_ in id_count:
        if id_count[id_] > 1:
            vals = [ps[i] for i in range(len(ids)) if ids[i] == id_]
            assert len(set(vals)) == 1
    for i in range(len(columns)):
        template[1+2*i] = (ids[i],encounters[ids[i]])
        if id_count[ids[i]] == 1 or ps[i] == -1:
            data[ids[i]].append(gen(columns[i],1-ps[i] if ps[i] != -1 else -1))
        else:
            if not ids[i] in start:
                p = random.random()*(1-ps[i])
                data[ids[i]].append(gen(columns[i],p))
                start[ids[i]] = p
            else:
                p=start[ids[i]]
                data[ids[i]].append(gen(columns[i],p+ps[i]))
        encounters[ids[i]] += 1
    for key in data:
        data[key].sort()
        data[key] = [format_data(x) for x in data[key]]
    for i in range(len(columns)):
        j,k = template[1+2*i]
        template[1+2*i] = data[j][k]
    print(f"PRAGMA profiling_output='{profiler_dir}.{_}.json';")
    print("".join(template))