bin_dir="build/release"
make clean
make STATS=EmptyAdditionalStats LARGE_ADDITIONAL_STATS=false && mv $bin_dir/duckdb $bin_dir/duckdb_min_max
make STATS=ClusterAdditionalStats LARGE_ADDITIONAL_STATS=false && mv $bin_dir/duckdb $bin_dir/duckdb_cluster
# make STATS=BloomAdditionalStats LARGE_ADDITIONAL_STATS=false && mv $bin_dir/duckdb $bin_dir/duckdb_bloom
# make STATS=DictionaryAdditionalStats LARGE_ADDITIONAL_STATS=false && mv $bin_dir/duckdb $bin_dir/duckdb_dictionary
make STATS=ClusterAdditionalStats LARGE_ADDITIONAL_STATS=true && mv $bin_dir/duckdb $bin_dir/duckdb_cluster_1000
# make STATS=BloomAdditionalStats LARGE_ADDITIONAL_STATS=true && mv $bin_dir/duckdb $bin_dir/duckdb_bloom_1000
# make STATS=DictionaryAdditionalStats LARGE_ADDITIONAL_STATS=true && mv $bin_dir/duckdb $bin_dir/duckdb_dictionary_1000