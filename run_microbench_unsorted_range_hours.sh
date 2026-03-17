dir="xperiments_results/unsorted_range_hours"
file="sql_queries_unsorted_range_hours.sql"
# dir="."
mkdir -p $dir/processed
cat template.csv > $dir/raw_data_min_max.csv
cat template.csv > $dir/raw_data_cluster.csv
cat template.csv > $dir/raw_data_cluster_1000.csv
rm test.db
duckdb -f $file > $dir/output_correct.txt
rm test.db
./build/release/duckdb_min_max -f $file 2>> $dir/raw_data_min_max.csv > $dir/output_min_max.txt
rm test.db
./build/release/duckdb_cluster -f $file 2>> $dir/raw_data_cluster.csv > $dir/output_cluster.txt
rm test.db
./build/release/duckdb_cluster_1000 -f $file 2>> $dir/raw_data_cluster_1000.csv > $dir/output_cluster_1000.txt
rm test.db
python3 data_processor.py 1 1000 0 $dir/raw_data_min_max.csv $dir/raw_data_cluster.csv $dir/raw_data_cluster_1000.csv
rm test.db
mv pruning.csv $dir/processed/
mv query_time.csv $dir/processed/
mv size.csv $dir/processed/
mv ingestion.csv $dir/processed/