#!/usr/bin/env bash

set -euo pipefail

if [[ $# -lt 2 ]]; then
    echo "Usage: $0 <sql_file> <output_folder>" >&2
    exit 1
fi

sql_file=$1
folder=$2
bin_dir="./duckdb/build/release"

if [[ ! -f "$sql_file" ]]; then
    echo "SQL file not found: $sql_file" >&2
    exit 1
fi
if [[ ! -x "$bin_dir/duckdb" ]]; then
    echo "DuckDB executable not found or not executable: $bin_dir/duckdb" >&2
    exit 1
fi

mkdir -p "$folder/log" "$folder/processed"

bloom_names=(
    BLOOM_SMALL
    BLOOM_MEDIUM
    BLOOM_LARGE
    BLOOM_SMALL_K2
    BLOOM_SMALL_K3
    BLOOM_MEDIUM_K2
    BLOOM_MEDIUM_K3
    BLOOM_LARGE_K1
    BLOOM_LARGE_K3
)
bloom_statistics=(5 6 7 10 11 12 13 14 15)

# Warm the query workload before recording any measurements.
rm -f test.db
"$bin_dir/duckdb" -statistic "${bloom_statistics[0]}" -f "$sql_file" &> /dev/null
rm -f test.db

processor_args=(1 1000 1)
for index in "${!bloom_names[@]}"; do
    name=${bloom_names[$index]}
    statistic=${bloom_statistics[$index]}
    log_file="$folder/log/${name,,}.log"

    cat template.csv > "$log_file"
    rm -f test.db
    "$bin_dir/duckdb" -statistic "$statistic" -f "$sql_file" 2>> "$log_file"
    rm -f test.db

    processor_args+=("$name" "$log_file")
done

python3 data_processor.py "${processor_args[@]}"
mv pruning.csv "$folder/processed/"
mv query_time.csv "$folder/processed/"
mv size.csv "$folder/processed/"
