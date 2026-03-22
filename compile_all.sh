bin_dir="build/release"
cd duckdb
make clean
make STATS=EmptyAdditionalStats ADDITIONAL_STATS_SCALE_LEVEL=0 && mv $bin_dir/duckdb $bin_dir/duckdb_min_max
make STATS=ClusterAdditionalStats ADDITIONAL_STATS_SCALE_LEVEL=0 && mv $bin_dir/duckdb $bin_dir/duckdb_cluster_10
make STATS=BloomAdditionalStats ADDITIONAL_STATS_SCALE_LEVEL=0 && mv $bin_dir/duckdb $bin_dir/duckdb_bloom_10
make STATS=DictionaryAdditionalStats ADDITIONAL_STATS_SCALE_LEVEL=0 && mv $bin_dir/duckdb $bin_dir/duckdb_dictionary_10
make STATS=ClusterAdditionalStats ADDITIONAL_STATS_SCALE_LEVEL=1 && mv $bin_dir/duckdb $bin_dir/duckdb_cluster_100
make STATS=BloomAdditionalStats ADDITIONAL_STATS_SCALE_LEVEL=1 && mv $bin_dir/duckdb $bin_dir/duckdb_bloom_100
make STATS=DictionaryAdditionalStats ADDITIONAL_STATS_SCALE_LEVEL=1 && mv $bin_dir/duckdb $bin_dir/duckdb_dictionary_100
make STATS=ClusterAdditionalStats ADDITIONAL_STATS_SCALE_LEVEL=2 && mv $bin_dir/duckdb $bin_dir/duckdb_cluster_1000
make STATS=BloomAdditionalStats ADDITIONAL_STATS_SCALE_LEVEL=2 && mv $bin_dir/duckdb $bin_dir/duckdb_bloom_1000
make STATS=DictionaryAdditionalStats ADDITIONAL_STATS_SCALE_LEVEL=2 && mv $bin_dir/duckdb $bin_dir/duckdb_dictionary_1000