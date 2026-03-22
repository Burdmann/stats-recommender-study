dir="experiments_results/$1"
file="$2"
num_tables=$3
num_queries=$4
num_repetitions=$5
# dir="."
mkdir -p $dir/processed
# cat template.csv > $dir/raw_data_min_max.csv
# cat template.csv > $dir/raw_data_cluster_10.csv
# cat template.csv > $dir/raw_data_bloom_10.csv
# cat template.csv > $dir/raw_data_dictionary_10.csv
# cat template.csv > $dir/raw_data_cluster_100.csv
# cat template.csv > $dir/raw_data_bloom_100.csv
# cat template.csv > $dir/raw_data_dictionary_100.csv
# cat template.csv > $dir/raw_data_cluster_1000.csv
# cat template.csv > $dir/raw_data_bloom_1000.csv
# cat template.csv > $dir/raw_data_dictionary_1000.csv
# rm test.db
# duckdb -f $file > $dir/output_correct.txt
# rm test.db
# ./duckdb/build/release/duckdb_min_max -f $file 2>> $dir/raw_data_min_max.csv > $dir/output_min_max.txt
# rm test.db
# ./duckdb/build/release/duckdb_cluster_10 -f $file 2>> $dir/raw_data_cluster_10.csv > $dir/output_cluster_10.txt
# rm test.db
# ./duckdb/build/release/duckdb_bloom_10 -f $file 2>> $dir/raw_data_bloom_10.csv > $dir/output_bloom_10.txt
# rm test.db
# ./duckdb/build/release/duckdb_dictionary_10 -f $file 2>> $dir/raw_data_dictionary_10.csv > $dir/output_dictionary_10.txt
# rm test.db
# ./duckdb/build/release/duckdb_cluster_100 -f $file 2>> $dir/raw_data_cluster_100.csv > $dir/output_cluster_100.txt
# rm test.db
# ./duckdb/build/release/duckdb_bloom_100 -f $file 2>> $dir/raw_data_bloom_100.csv > $dir/output_bloom_100.txt
# rm test.db
# ./duckdb/build/release/duckdb_dictionary_100 -f $file 2>> $dir/raw_data_dictionary_100.csv > $dir/output_dictionary_100.txt
# rm test.db
# ./duckdb/build/release/duckdb_cluster_1000 -f $file 2>> $dir/raw_data_cluster_1000.csv > $dir/output_cluster_1000.txt
# rm test.db
# ./duckdb/build/release/duckdb_bloom_1000 -f $file 2>> $dir/raw_data_bloom_1000.csv > $dir/output_bloom_1000.txt
# rm test.db
# ./duckdb/build/release/duckdb_dictionary_1000 -f $file 2>> $dir/raw_data_dictionary_1000.csv > $dir/output_dictionary_1000.txt
rm test.db
python3 data_processor.py $num_tables $num_queries $num_repetitions "Min/max" $dir/raw_data_min_max.csv "Min/max clusters (10x)" $dir/raw_data_cluster_10.csv "Bloom filter (10x)" $dir/raw_data_bloom_10.csv "Dictionary (10x)" $dir/raw_data_dictionary_10.csv "Min/max clusters (100x)" $dir/raw_data_cluster_100.csv "Bloom filter (100x)" $dir/raw_data_bloom_100.csv "Dictionary (100x)" $dir/raw_data_dictionary_100.csv "Min/max clusters (1000x)" $dir/raw_data_cluster_1000.csv "Bloom filter (1000x)" $dir/raw_data_bloom_1000.csv "Dictionary (1000x)" $dir/raw_data_dictionary_1000.csv
rm test.db
mv pruning.csv $dir/processed/
mv query_time.csv $dir/processed/
mv size.csv $dir/processed/
mv ingestion.csv $dir/processed/