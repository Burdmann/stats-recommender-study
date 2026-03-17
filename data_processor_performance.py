import duckdb
import sys
import os

NUM_TABLES = int(sys.argv[1])
NUM_QUERIES = int(sys.argv[2])
NUM_ITERATIONS = int(sys.argv[3])
in_files = sys.argv[4:]

ATTACHED = True # FIXME:
NUM_STATS = len(in_files)
QUERIES_START = 2*NUM_TABLES+2 + (2 if ATTACHED else 0)
# names = ["Min/max","Min/max clusters","Bloom filter"]
names = ["Min/max","Min/max clusters (10x)","Min/max clusters (100x)","Min/max clusters (1000x)"]
# names = ["R50","R100","R200","R400","C50","C100","C200","C400"]
# names = ["Min/max","Min/max clusters","Bloom filter","Prune everything"]
# names = ["200","500","1K","2K","5K","10K"]
# names = ["100","200","500","1K","2K","5K"]

times = [[] for _ in range(NUM_STATS)]

if os.path.exists("test.db"):
    os.remove("test.db")
duckdb.execute("ATTACH 'test.db'")
duckdb.execute("USE test")
duckdb.execute(f"CREATE TABLE Result_time (Name VARCHAR, Unit VARCHAR, Query INT, {", ".join([f'"{name}" FLOAT' for name in names])});")
for idx,in_file in enumerate(in_files):
    duckdb.execute(f"CREATE TABLE tbl AS SELECT * FROM read_csv('{in_file}',max_line_size=100000000);")
    duckdb.execute("CREATE TABLE times AS SELECT start_time,end_time,t1.CommandID AS CommandID FROM (SELECT Time as end_time,CommandID FROM tbl WHERE type = 'SQL_COMMAND_RUN_END') t1 JOIN (SELECT Time as start_time,CommandID FROM tbl WHERE type = 'SQL_COMMAND_RUN_START') t2 on t1.CommandID = t2.commandID;")
    for i in range(NUM_QUERIES):
        times[idx] += [l[0] for l in duckdb.execute(f"SELECT ROUND((end_time-start_time)/1000000.0,5) FROM times WHERE CommandID >= {QUERIES_START+NUM_ITERATIONS*i} AND CommandID < {QUERIES_START+NUM_ITERATIONS*(1+i)};").fetchall()]
    duckdb.execute(f"DROP TABLE tbl;")
    duckdb.execute(f"DROP TABLE times;")
for i in range(NUM_QUERIES):
    for j in range(NUM_ITERATIONS):
        idx = i*NUM_ITERATIONS+j
        duckdb.execute(f"INSERT INTO Result_time VALUES ('Time','s',{i+1},{",".join([str(time[idx]) for time in times])});")
duckdb.execute("COPY Result_time TO 'query_time.csv';")