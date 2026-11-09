import duckdb
import sys
import os

table = sys.argv[1]
# if os.path.exists("test.db"):
#     os.remove("test.db")
duckdb.execute("ATTACH 'test.db'")
duckdb.execute("USE test")
# duckdb.execute(f"CREATE OR REPLACE TABLE tbl AS SELECT * FROM read_csv('{table}',sample_size=-1);")

for col,*_ in duckdb.execute(f"DESCRIBE tbl;").fetchall():
    mn,mx,distinct = duckdb.execute(f'SELECT MIN("{col}"),MAX("{col}"),approx_count_distinct("{col}") FROM tbl;').fetchall()[0]
    print(f'"{col.strip()}", ({distinct} distinct values)')
os.remove("test.db")