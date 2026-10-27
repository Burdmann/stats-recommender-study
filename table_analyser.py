import duckdb
import sys

table = sys.argv[1]

duckdb.execute(f"CREATE TABLE tbl AS SELECT * FROM '{table}';")

for col,*_ in duckdb.execute(f"DESCRIBE tbl;").fetchall():
    mn,mx,distinct = duckdb.execute(f'SELECT MIN("{col}"),MAX("{col}"),COUNT(DISTINCT "{col}") FROM tbl;').fetchall()[0]
    print(f'"{col}","{mn}","{mx}" ({distinct} distinct values)')