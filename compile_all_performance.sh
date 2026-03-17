bin_dir="build/release"
make STATS=EmptyAdditionalStats LARGE_ADDITIONAL_STATS=false && mv $bin_dir/duckdb $bin_dir/duckdb_min_max_performance
make STATS=ClusterAdditionalStats LARGE_ADDITIONAL_STATS=false && mv $bin_dir/duckdb $bin_dir/duckdb_cluster_performance
# make STATS=BloomAdditionalStats LARGE_ADDITIONAL_STATS=false && mv $bin_dir/duckdb $bin_dir/duckdb_bloom_performance
# make STATS=DictionaryAdditionalStats LARGE_ADDITIONAL_STATS=false && mv $bin_dir/duckdb $bin_dir/duckdb_dictionary_performance
make STATS=ClusterAdditionalStats LARGE_ADDITIONAL_STATS=true && mv $bin_dir/duckdb $bin_dir/duckdb_cluster_1000_performance
# make STATS=BloomAdditionalStats LARGE_ADDITIONAL_STATS=true && mv $bin_dir/duckdb $bin_dir/duckdb_bloom_1000_performance
# make STATS=DictionaryAdditionalStats LARGE_ADDITIONAL_STATS=true && mv $bin_dir/duckdb $bin_dir/duckdb_dictionary_1000_performance