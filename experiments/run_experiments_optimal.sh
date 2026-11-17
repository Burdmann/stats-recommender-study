#!/bin/bash
# Pruning of the near-optimal statistics (nullspace for point queries, occupancy for range queries) next
# to min/max and Bloom filters, on the unsorted micro benchmark. Run from the repository root after
# compile_all.sh (binaries in duckdb/build/release/duckdb_<name>) and with unsorted.csv in the root.
#
# Usage: ./experiments/run_experiments_optimal.sh [parallel jobs, default 2] [binaries to run, default all]
# Every (query file, binary) pair runs in its own directory below experiments_results/optimal, because
# the SQL files create test.db in the working directory. Results: experiments_results/optimal/pruning.csv
jobs=${1:-2}
root=$(pwd)
out="$root/experiments_results/optimal"
mkdir -p "$out"

point=experiments/sql/sortedness/sql_queries_unsorted.sql
runs=""
for bin in min_max bloom_10 bloom_100 bloom_1000 nullspace_10 nullspace_100 nullspace_1000 occupancy_10 occupancy_100 occupancy_1000; do
	runs+="unsorted $point $bin"$'\n'
done
for w in seconds minutes hours; do
	# Bloom filters do not answer range queries (QueryRange never prunes), so they equal min/max here
	for bin in min_max occupancy_10 occupancy_100 occupancy_1000; do
		runs+="unsorted_range_$w experiments/sql/range/sql_queries_unsorted_range_$w.sql $bin"$'\n'
	done
done

run_one() {
	set -e
	local name=$1 file=$2 bin=$3 root=$4 out=$5
	local dir="$out/$name/$bin"
	rm -rf "$dir" && mkdir -p "$dir" && cd "$dir"
	ln -s "$root/unsorted.csv" unsorted.csv
	cat "$root/template.csv" > raw_data.csv
	"$root/duckdb/build/release/duckdb_$bin" -f "$root/$file" 2>> raw_data.csv > output.txt
	rm -f test.db test.db.wal unsorted.csv
	echo "$(date +%T) done $name $bin"
}
export -f run_one
if [ -n "$2" ]; then
	runs=$(printf "%s" "$runs" | grep -wE "$(echo $2 | tr ' ,' '||')")$'\n'
fi
printf "%s" "$runs" | xargs -P "$jobs" -L 1 bash -c 'run_one "$0" "$1" "$2" "'"$root"'" "'"$out"'"'

python3 experiments/summarize_optimal.py "$out"
