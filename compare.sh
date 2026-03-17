rm test.db
duckdb -f sql_queries_tpch.sql > correct.txt
rm test.db
./build/release/duckdb -f sql_queries_tpch.sql 2> /dev/null > wrong.txt
rm test.db
diff correct.txt wrong.txt