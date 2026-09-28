import duckdb
import sys
import os

NUM_TABLES = int(sys.argv[1])
NUM_QUERIES = int(sys.argv[2])
NUM_ITERATIONS = int(sys.argv[3])
OUT_FILE = sys.argv[4]
rest = sys.argv[5:]
names,in_files = rest[::2],rest[1::2]

ATTACHED = True # FIXME:
NUM_STATS = len(in_files)
QUERIES_START = 2*NUM_TABLES+NUM_QUERIES+2 + (2 if ATTACHED else 0)

if os.path.exists("test.db"):
    os.remove("test.db")
duckdb.execute("ATTACH 'test.db'")
duckdb.execute("USE test")
duckdb.execute(f"CREATE TABLE result (table_name VARCHAR, rowgroup INT, col INT, statistic VARCHAR, size HUGEINT, pruning_power FLOAT);")
for idx,(in_file,name) in enumerate(zip(in_files,names)):
    duckdb.execute(f"CREATE OR REPLACE TABLE tbl AS SELECT * FROM read_csv('{in_file}',max_line_size=100000000);")
    duckdb.execute("CREATE OR REPLACE TABLE sizes AS SELECT Data->>'table_id' AS table_name,CAST(Data->'rowgroup' AS INT) AS rowgroup,CAST(Data->'column' AS INT) AS col,CAST(Data->'size' AS HUGEINT) AS size FROM tbl WHERE Type='END_INITIALISE_ADDITIONAL_STATS';")

    duckdb.execute(f"INSERT INTO result SELECT sizes.table_name AS table_name, sizes.rowgroup AS rowgroup,sizes.col AS col,statistic,size,pruning_power FROM (sizes JOIN (SELECT Data->>'table_id' AS table_name, CAST(Data->'rowgroup' AS INT) AS rowgroup,CAST(Data->'column' AS INT) AS col,'{name}' AS statistic, countif(CAST(Data->'result' AS INT) = 2)/count(*) AS pruning_power FROM tbl WHERE Type='EVAL_STATISTICS' GROUP BY (table_name,rowgroup,col)) t2 ON sizes.rowgroup = t2.rowgroup AND sizes.col = t2.col AND sizes.table_name = t2.table_name);")

duckdb.execute(f"COPY result TO '{OUT_FILE}';")