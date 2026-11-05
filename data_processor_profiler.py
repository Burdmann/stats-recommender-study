import duckdb
import sys
import os
from collections import defaultdict
import json

NUM_TABLES = int(sys.argv[1])
NUM_QUERIES = int(sys.argv[2])
NUM_ITERATIONS = int(sys.argv[3])
rest = sys.argv[4:]
names,in_folders = rest[::2],rest[1::2]

def avg(l):
    if not l:
        return 0
    return sum(l)/len(l)

NUM_STATS = len(in_folders)

times = [defaultdict(list) for _ in range(NUM_STATS)]
stats_eval_times = [defaultdict(list) for _ in range(NUM_STATS)]
rows_scanned = [defaultdict(list) for _ in range(NUM_STATS)]
sizes = [[] for _ in range(NUM_STATS)]
max_sizes = [[] for _ in range(NUM_STATS)]
# tables = [[] for _ in range(NUM_STATS)]

queries = set()
for i,(stat,dir) in enumerate(zip(names,in_folders)):
    for file in os.listdir(dir):
        if not file.endswith(".json"):
            continue
        d=json.load(open(f"{dir}/{file}"))
        query = file.split(".")[0]
        queries.add(query)

        time,stats_eval_time,scanned = d["cpu_time"],d["cumulative_custom_statistics_time"],d["cumulative_actual_rows_scanned"]
        times[i][query].append(time)
        stats_eval_times[i][query].append(stats_eval_time)
        rows_scanned[i][query].append(scanned)


duckdb.execute(f"CREATE TABLE Result_time (Name VARCHAR, Unit VARCHAR, Query VARCHAR, {", ".join([f'"{name}" DOUBLE' for name in names])});")
duckdb.execute(f"CREATE TABLE Result_pruning (Name VARCHAR, Unit VARCHAR, Query VARCHAR, {", ".join([f'"{name}" DOUBLE' for name in names])});")
duckdb.execute(f"CREATE TABLE Result_size (Name VARCHAR, Unit VARCHAR, Table_name VARCHAR, {", ".join([f'"{name}" INT64' for name in names])});")
for query in queries:
    duckdb.execute(f"INSERT INTO Result_time VALUES ('Time','s','{query}',{",".join([str(avg(time[query])) for time in times])});")
    duckdb.execute(f"INSERT INTO Result_pruning VALUES ('Partitions scanned',NULL,'{query}',{",".join([str(avg(scanned[query])) for scanned in rows_scanned])});")
for i in range(NUM_TABLES):
    pass
    # if (len(set([tables[j][i] for j in range(NUM_STATS)]))) != 1:
    #     raise "tables are not in the same order in given input files"
    # duckdb.execute(f"INSERT INTO Result_size VALUES ('Size','B','{tables[0][i]}',{",".join([str(size[i]) for size in sizes])});")
    # duckdb.execute(f"INSERT INTO Result_size VALUES ('Max Size','B','{tables[0][i]}',{",".join([str(size[i]) for size in max_sizes])});")

duckdb.execute("COPY Result_time TO 'query_time.csv';")
duckdb.execute("COPY Result_pruning TO 'pruning.csv';")
duckdb.execute("COPY Result_size TO 'size.csv';")
# duckdb.execute("COPY Result_ingestion TO 'ingestion.csv';")