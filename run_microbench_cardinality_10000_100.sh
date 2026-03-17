dir="xperiments_results/cardinality_10000"
file="sql_queries_cardinality_10000_100.sql"
# dir="."
mkdir -p $dir/processed
# cat template.csv > $dir/raw_data_min_max.csv
# cat template.csv > $dir/raw_data_cluster.csv
# cat template.csv > $dir/raw_data_bloom.csv
# cat template.csv > $dir/raw_data_dictionary.csv
# cat template.csv > $dir/raw_data_cluster_1000.csv
# cat template.csv > $dir/raw_data_bloom_1000.csv
# cat template.csv > $dir/raw_data_dictionary_1000.csv
# rm test.db
# duckdb -f $file > $dir/output_correct.txt
# rm test.db
# ./build/release/duckdb_min_max -f $file 2>> $dir/raw_data_min_max.csv > $dir/output_min_max.txt
# rm test.db
# ./build/release/duckdb_cluster -f $file 2>> $dir/raw_data_cluster.csv > $dir/output_cluster.txt
# rm test.db
# ./build/release/duckdb_bloom -f $file 2>> $dir/raw_data_bloom.csv > $dir/output_bloom.txt
# rm test.db
# ./build/release/duckdb_dictionary -f $file 2>> $dir/raw_data_dictionary.csv > $dir/output_dictionary.txt
# rm test.db
# ./build/release/duckdb_cluster_1000 -f $file 2>> $dir/raw_data_cluster_1000.csv > $dir/output_cluster_1000.txt
# rm test.db
# ./build/release/duckdb_bloom_1000 -f $file 2>> $dir/raw_data_bloom_1000.csv > $dir/output_bloom_1000.txt
# rm test.db
# ./build/release/duckdb_dictionary_1000 -f $file 2>> $dir/raw_data_dictionary_1000.csv > $dir/output_dictionary_1000.txt
rm test.db
python3 data_processor.py 1 100 0 $dir/raw_data_min_max.csv $dir/raw_data_cluster.csv $dir/raw_data_bloom.csv $dir/raw_data_dictionary.csv $dir/raw_data_cluster_1000.csv $dir/raw_data_bloom_1000.csv $dir/raw_data_dictionary_1000.csv
rm test.db
mv pruning.csv $dir/processed/
mv query_time.csv $dir/processed/
mv size.csv $dir/processed/
mv ingestion.csv $dir/processed/