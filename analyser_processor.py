import duckdb
import sys
import os

OUT_FILE = sys.argv[1]
rest = sys.argv[2:]
names,in_files = rest[::2],rest[1::2]

EMPTY_SIZE = 0

if os.path.exists("test.db"):
    os.remove("test.db")
duckdb.execute("ATTACH 'test.db'")
duckdb.execute("USE test")
duckdb.execute(f"CREATE TABLE result (table_name VARCHAR, rowgroup INT, col INT, statistic INT,statistic_alias VARCHAR, size HUGEINT, pruning_power INT);")
for idx,(in_file,name) in enumerate(zip(in_files,names)):
    duckdb.execute(f"CREATE OR REPLACE TABLE tbl AS SELECT * FROM read_csv('{in_file}',max_line_size=100000000);")
    duckdb.execute("CREATE OR REPLACE TABLE sizes AS SELECT Data->>'table_id' AS table_name,CAST(Data->'rowgroup' AS INT) AS rowgroup,CAST(Data->'column' AS INT) AS col,CAST(Data->'size' AS HUGEINT) AS size FROM tbl WHERE Type='END_INITIALISE_ADDITIONAL_STATS';")

    duckdb.execute(f"""
                    INSERT INTO result
                    SELECT sizes.table_name AS table_name, sizes.rowgroup AS rowgroup,sizes.col AS col,{idx} as statistic,'{name}' AS statistic_alias,size,COALESCE(pruning_power,0)
                    FROM (
                        sizes LEFT JOIN (
                            SELECT Data->>'table_id' AS table_name, CAST(Data->'rowgroup' AS INT) AS rowgroup,CAST(Data->'column' AS INT) AS col, countif(CAST(Data->'result' AS INT) = 2) AS pruning_power
                            FROM tbl
                            WHERE Type='EVAL_STATISTICS'
                            GROUP BY (table_name,rowgroup,col)
                        ) t2
                        ON sizes.rowgroup = t2.rowgroup AND sizes.col = t2.col AND sizes.table_name = t2.table_name
                    );""")

duckdb.execute(f"COPY result TO '{OUT_FILE}';")