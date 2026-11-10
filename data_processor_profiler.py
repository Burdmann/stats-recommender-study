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

# cpu_times = [defaultdict(list) for _ in range(NUM_STATS)]
total_times = [defaultdict(list) for _ in range(NUM_STATS)]
stats_eval_times = [defaultdict(list) for _ in range(NUM_STATS)]
rows_scanned = [defaultdict(list) for _ in range(NUM_STATS)]
rows_in_qualifying_partitions = [defaultdict(list) for _ in range(NUM_STATS)]
sizes = [{} for _ in range(NUM_STATS)]
max_sizes = [{} for _ in range(NUM_STATS)]
tables = [set() for _ in range(NUM_STATS)]


queries = set()
for i,(stat,dir) in enumerate(zip(names,in_folders)):
    for file in os.listdir(f"{dir}/tables"):
        if not file.endswith(".json"):
            continue
        d=json.load(open(f"{dir}/tables/{file}"))
        table = file[:-5]
        tables[i].add(table)
        for child in d['children']:
            if 'statistics_size' in child:
                break
        else:
            print("no child in the profiler file contained a 'statistics_size' field")
            exit(-1)
        table_sizes = [x['size_bytes'] for x in child['statistics_size']]
        max_sizes[i][table] = max(table_sizes)
        sizes[i][table] = sum(table_sizes)
    for file in os.listdir(dir):
        if not file.endswith(".json"):
            continue
        d=json.load(open(f"{dir}/{file}"))
        query = file.split(".")[0]
        queries.add(query)

        time,stats_eval_time,scanned = d["latency"],d["cumulative_custom_statistics_time"],d["cumulative_actual_rows_scanned"]
        total_times[i][query].append(time)
        # cpu_times[i][query].append(cpu_time)
        stats_eval_times[i][query].append(stats_eval_time)
        rows_scanned[i][query].append(scanned)
        rows_in_qualifying_partitions[i][query].append(d["cumulative_rows_in_qualifying_partitions"])


duckdb.execute(f"CREATE TABLE Result_time (Measurement VARCHAR, Unit VARCHAR, Query VARCHAR, {", ".join([f'"{name}" DOUBLE' for name in names])});")
# duckdb.execute(f"CREATE TABLE Result_cpu_time (Measurement VARCHAR, Unit VARCHAR, Query VARCHAR, {", ".join([f'"{name}" DOUBLE' for name in names])});")
duckdb.execute(f"CREATE TABLE Result_stats_eval_time (Measurement VARCHAR, Unit VARCHAR, Query VARCHAR, {", ".join([f'"{name}" DOUBLE' for name in names])});")
duckdb.execute(f"CREATE TABLE Result_pruning (Measurement VARCHAR, Unit VARCHAR, Query VARCHAR, {", ".join([f'"{name}" DOUBLE' for name in names])});")
duckdb.execute(f"CREATE TABLE Result_contributing_rows (Measurement VARCHAR, Unit VARCHAR, Query VARCHAR, {", ".join([f'"{name}" DOUBLE' for name in names])});")
duckdb.execute(f"CREATE TABLE Result_size (Measurement VARCHAR, Unit VARCHAR, Table_name VARCHAR, {", ".join([f'"{name}" INT64' for name in names])});")
for query in queries:
    duckdb.execute(f"INSERT INTO Result_time VALUES ('Time','s','{query}',{",".join([str(avg(time[query])) for time in total_times])});")
    # duckdb.execute(f"INSERT INTO Result_cpu_time VALUES ('Time','s','{query}',{",".join([str(avg(time[query])) for time in cpu_times])});")
    duckdb.execute(f"INSERT INTO Result_stats_eval_time VALUES ('Time','s','{query}',{",".join([str(avg(time[query])) for time in stats_eval_times])});")
    duckdb.execute(f"INSERT INTO Result_pruning VALUES ('Partitions scanned',NULL,'{query}',{",".join([str(avg(scanned[query])) for scanned in rows_scanned])});")
    duckdb.execute(f"INSERT INTO Result_contributing_rows VALUES ('Rows in qualifying partitions','rows','{query}',{",".join([str(avg(rows[query])) for rows in rows_in_qualifying_partitions])});")
for table in tables[0]:
    # if (len(set([tables[j][i] for j in range(NUM_STATS)]))) != 1:
    #     raise "tables are not in the same order in given input files"
    duckdb.execute(f"INSERT INTO Result_size VALUES ('Size','B','{table}',{",".join([str(size[table]) for size in sizes])});")
    duckdb.execute(f"INSERT INTO Result_size VALUES ('Max Size','B','{table}',{",".join([str(size[table]) for size in max_sizes])});")

duckdb.execute("CREATE OR REPLACE TABLE Result_time AS SELECT * FROM Result_time ORDER BY CAST(REGEXP_EXTRACT(Query, '^q([0-9]+)', 1) AS INTEGER), COALESCE(TRY_CAST(REGEXP_EXTRACT(Query, '_([0-9]+)$', 1) AS INTEGER),999999)")
duckdb.execute("CREATE OR REPLACE TABLE Result_stats_eval_time AS SELECT * FROM Result_stats_eval_time ORDER BY CAST(REGEXP_EXTRACT(Query, '^q([0-9]+)', 1) AS INTEGER), COALESCE(TRY_CAST(REGEXP_EXTRACT(Query, '_([0-9]+)$', 1) AS INTEGER),999999)")
duckdb.execute("CREATE OR REPLACE TABLE Result_pruning AS SELECT * FROM Result_pruning ORDER BY CAST(REGEXP_EXTRACT(Query, '^q([0-9]+)', 1) AS INTEGER), COALESCE(TRY_CAST(REGEXP_EXTRACT(Query, '_([0-9]+)$', 1) AS INTEGER),999999)")
duckdb.execute("CREATE OR REPLACE TABLE Result_contributing_rows AS SELECT * FROM Result_contributing_rows ORDER BY CAST(REGEXP_EXTRACT(Query, '^q([0-9]+)', 1) AS INTEGER), COALESCE(TRY_CAST(REGEXP_EXTRACT(Query, '_([0-9]+)$', 1) AS INTEGER),999999)")

duckdb.execute("COPY Result_time TO 'total_time.csv';")
duckdb.execute("COPY Result_stats_eval_time TO 'stats_eval_time.csv';")
# duckdb.execute("COPY Result_cpu_time TO 'cpu_time.csv';")
duckdb.execute("COPY Result_pruning TO 'pruning.csv';")
duckdb.execute("COPY Result_contributing_rows TO 'contributing_rows.csv';")
duckdb.execute("COPY Result_size TO 'size.csv';")
# duckdb.execute("COPY Result_ingestion TO 'ingestion.csv';")