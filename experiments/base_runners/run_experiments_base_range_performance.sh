dir="experiments_results/$1"
file="$2"
num_tables=$3
num_queries=$4
num_repetitions=$5
# dir="."
mkdir -p $dir/processed
cat template.csv > $dir/raw_data_min_max.csv
cat template.csv > $dir/raw_data_cluster_10.csv
cat template.csv > $dir/raw_data_cluster_100.csv
cat template.csv > $dir/raw_data_cluster_1000.csv
rm test.db
duckdb -f $file > $dir/output_correct.txt
rm test.db
./duckdb/build/release/duckdb_min_max -f $file 2>> $dir/raw_data_min_max.csv > $dir/output_min_max.txt
rm test.db
./duckdb/build/release/duckdb_cluster_10 -f $file 2>> $dir/raw_data_cluster_10.csv > $dir/output_cluster_10.txt
rm test.db
./duckdb/build/release/duckdb_cluster_100 -f $file 2>> $dir/raw_data_cluster_100.csv > $dir/output_cluster_100.txt
rm test.db
./duckdb/build/release/duckdb_cluster_1000 -f $file 2>> $dir/raw_data_cluster_1000.csv > $dir/output_cluster_1000.txt
rm test.db
python3 data_processor_performance.py $num_tables $num_queries $num_repetitions "Min/max" $dir/raw_data_min_max.csv "Min/max clusters (10x)" $dir/raw_data_cluster_10.csv "Min/max clusters (100x)" $dir/raw_data_cluster_100.csv "Min/max clusters (1000x)" $dir/raw_data_cluster_1000.csv
rm test.db
mv pruning.csv $dir/processed/
mv query_time.csv $dir/processed/
mv size.csv $dir/processed/
mv ingestion.csv $dir/processed/