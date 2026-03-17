import duckdb
import sys
import os

NUM_TABLES = int(sys.argv[1])
NUM_QUERIES = int(sys.argv[2])
NUM_ITERATIONS = int(sys.argv[3])
in_files = sys.argv[4:]

ATTACHED = True # FIXME:
NUM_STATS = len(in_files)
QUERIES_START = 2*NUM_TABLES+NUM_QUERIES+2 + (2 if ATTACHED else 0)
# names = ["Min/max","Min/max clusters","Bloom filter"]
names = ["Min/max","Min/max clusters","Bloom filter","Dictionary","Min/max clusters (1000x)","Bloom filter (1000x)","Dictionary (1000x)"]
names = ["Min/max","Min/max clusters","Min/max clusters (1000x)"]
# names = ["R50","R100","R200","R400","C50","C100","C200","C400"]
# names = ["Min/max","Min/max clusters","Bloom filter","Prune everything"]
# names = ["200","500","1K","2K","5K","10K"]
# names = ["100","200","500","1K","2K","5K"]

times = [[] for _ in range(NUM_STATS)]
rows_scanned = [[] for _ in range(NUM_STATS)]
sizes = [[] for _ in range(NUM_STATS)]
max_sizes = [[] for _ in range(NUM_STATS)]
ingestion_times = [[] for _ in range(NUM_STATS)]
tables = [[] for _ in range(NUM_STATS)]

if os.path.exists("test.db"):
    os.remove("test.db")
duckdb.execute("ATTACH 'test.db'")
duckdb.execute("USE test")
duckdb.execute(f"CREATE TABLE Result_time (Name VARCHAR, Unit VARCHAR, Query INT, {", ".join([f'"{name}" FLOAT' for name in names])});")
duckdb.execute(f"CREATE TABLE Result_pruning (Name VARCHAR, Unit VARCHAR, Query INT, {", ".join([f'"{name}" INT64' for name in names])});")
duckdb.execute(f"CREATE TABLE Result_size (Name VARCHAR, Unit VARCHAR, Table_name VARCHAR, {", ".join([f'"{name}" INT64' for name in names])});")
duckdb.execute(f"CREATE TABLE Result_ingestion (Name VARCHAR, Unit VARCHAR, Table_name VARCHAR, {", ".join([f'"{name}" FLOAT' for name in names])});")
for idx,in_file in enumerate(in_files):
    duckdb.execute(f"CREATE TABLE tbl AS SELECT * FROM read_csv('{in_file}',max_line_size=100000000);")
    duckdb.execute("CREATE TABLE times AS SELECT start_time,end_time,t1.CommandID AS CommandID FROM (SELECT Time as end_time,CommandID FROM tbl WHERE type = 'SQL_COMMAND_RUN_END') t1 JOIN (SELECT Time as start_time,CommandID FROM tbl WHERE type = 'SQL_COMMAND_RUN_START') t2 on t1.CommandID = t2.commandID;")

    for i in range(NUM_TABLES):
        sizes[idx].append(duckdb.execute(f"SELECT COALESCE(SUM(CAST(Data->'size' AS HUGEINT)),0) FROM tbl WHERE type = 'END_INITIALISE_ADDITIONAL_STATS' AND CommandID = {NUM_TABLES+i+(3 if ATTACHED else 1)};").fetchone()[0])
        max_sizes[idx].append(duckdb.execute(f"SELECT COALESCE(MAX(CAST(Data->'size' AS HUGEINT)),0) FROM tbl WHERE type = 'END_INITIALISE_ADDITIONAL_STATS' AND CommandID = {NUM_TABLES+i+(3 if ATTACHED else 1)};").fetchone()[0])
        ingestion_times[idx].append(duckdb.execute(f"SELECT ROUND((end_time-start_time)/1000000.0,5) FROM times WHERE CommandID = {NUM_TABLES+i+(3 if ATTACHED else 1)}").fetchone()[0])
        tables[idx].append(duckdb.execute(f"SELECT CAST(Data->'command' AS VARCHAR) FROM tbl WHERE type='SQL_COMMAND_RUN_START' AND CommandID = {NUM_TABLES+i+(3 if ATTACHED else 1)}").fetchone()[0].split()[1])

    for i in range(NUM_QUERIES):
        times[idx] += [l[0] for l in duckdb.execute(f"SELECT ROUND((end_time-start_time)/1000000.0,5) FROM times WHERE CommandID >= {QUERIES_START+NUM_ITERATIONS*i} AND CommandID < {QUERIES_START+NUM_ITERATIONS*(1+i)};").fetchall()]
        rows_scanned[idx].append(duckdb.execute(f"SELECT SUM(CAST(Data->'count' AS HUGEINT)) AS val FROM tbl WHERE type = 'SCANNED_PARTITIONS' and CommandID = {2*NUM_TABLES+i+(3 if ATTACHED else 1)};").fetchone()[0])
    duckdb.execute(f"DROP TABLE tbl;")
    duckdb.execute(f"DROP TABLE times;")
for i in range(NUM_QUERIES):
    for j in range(NUM_ITERATIONS):
        idx = i*NUM_ITERATIONS+j
        duckdb.execute(f"INSERT INTO Result_time VALUES ('Time','s',{i+1},{",".join([str(time[idx]) for time in times])});")
    duckdb.execute(f"INSERT INTO Result_pruning VALUES ('Partitions scanned',NULL,{i+1},{",".join([str(scanned[i]) for scanned in rows_scanned])});")
for i in range(NUM_TABLES):
    if (len(set([tables[j][i] for j in range(NUM_STATS)]))) != 1:
        raise "tables are not in the same order in given input files"
    duckdb.execute(f"INSERT INTO Result_size VALUES ('Size','B','{tables[0][i]}',{",".join([str(size[i]) for size in sizes])});")
    duckdb.execute(f"INSERT INTO Result_size VALUES ('Max Size','B','{tables[0][i]}',{",".join([str(size[i]) for size in max_sizes])});")
    duckdb.execute(f"INSERT INTO Result_ingestion VALUES ('Ingestion','s','{tables[0][i]}',{",".join([str(time[i]) for time in ingestion_times])});")
duckdb.execute("COPY Result_time TO 'query_time.csv';")
duckdb.execute("COPY Result_pruning TO 'pruning.csv';")
duckdb.execute("COPY Result_size TO 'size.csv';")
duckdb.execute("COPY Result_ingestion TO 'ingestion.csv';")