sql_file=$1
folder=$2
base_size=$3
num_tables=$4
num_queries=$5
num_iterations=$6
bin_dir="./duckdb/build/release"

# ---------------------------- EXAMPLES ---------------------------------
# ./run_experiments.sh sql_queries_combined.sql combined_experiment/ 390656 1 1000 1
# ./run_experiments.sh sql_queries_yellow_tripdata_2024.sql taxi_experiment/ 1171504 1 15 100
# ./run_experiments.sh sql_queries_flights.sql experiment_results/flights 392272 1 26 100
# ./run_experiments.sh sql_queries_flights_line_plot.sql experiment_results/flights_line_plot 392272 1 32 100
# ./run_experiments.sh sql_queries_github.sql experiment_results/github 11596739 1 30 100
# ./run_experiments.sh sql_queries_github_line_plot.sql experiment_results/github_line_plot 11596739 1 20 100
# -----------------------------------------------------------------------

# mkdir -p $folder/1x
# mkdir -p $folder/10x
# mkdir -p $folder/50x
# mkdir -p $folder/200x
# for size in "1x" "10x" "50x" "200x"; do
#         mkdir -p $folder/log//profiler$size/cluster
#         mkdir -p $folder/log//profiler$size/bloom
#         mkdir -p $folder/log//profiler$size/greedy_simple
#         mkdir -p $folder/log//profiler$size/greedy
# done
# mkdir -p $folder/processed

# rm test.db
# ./analyser.sh $folder/analysis $sql_file

# for i in 1 10 50 200; do
#         python3 assigners/greedy_simple.py $folder/analysis/final.csv $folder/$i"x"/assignment_simple.csv $(($base_size * $i))
#         python3 assigners/greedy.py $folder/analysis/final.csv $folder/$i"x"/assignment.csv $(($base_size * $i))
# done

# run_inner() {
#         mkdir -p profiler/tables
#         $1
#         rm -r $folder/log/$2
#         mkdir -p $folder/log/$2
#         mv profiler $folder/log/$2
#         rm test.db
# }

# run_adaptive() {
#         run_inner "$bin_dir/duckdb -statistic-file $1 -f $sql_file" $2
# }
# run() {
#         run_inner "$bin_dir/duckdb -statistic $1 -f $sql_file" $2
# }

# # this is just to avoid a cold run, the data from this run is thrown away
# mkdir profiler
# $bin_dir/duckdb -statistic 9 -f $sql_file
# rm test.db
# rm -r profiler


# run 1 "1x/cluster"
# run 9 "1x/bloom"
# run_adaptive $folder/1x/assignment_simple.csv "1x/greedy_simple"
# run_adaptive $folder/1x/assignment.csv "1x/greedy"

# run 2 "10x/cluster"
# run 5 "10x/bloom"
# run_adaptive $folder/10x/assignment_simple.csv "10x/greedy_simple"
# run_adaptive $folder/10x/assignment.csv "10x/greedy"

# run 3 "50x/cluster"
# run 6 "50x/bloom"
# run_adaptive $folder/50x/assignment_simple.csv "50x/greedy_simple"
# run_adaptive $folder/50x/assignment.csv "50x/greedy"

# run 4 "200x/cluster"
# run 7 "200x/bloom"
# run_adaptive $folder/200x/assignment_simple.csv "200x/greedy_simple"
# run_adaptive $folder/200x/assignment.csv "200x/greedy"

python3 data_processor_profiler.py $num_tables $num_queries $num_iterations MIN_MAX $folder/log/1x/cluster/profiler BLOOM_1 $folder/log/1x/bloom/profiler GREEDY_SIMPLE_1 $folder/log/1x/greedy_simple/profiler GREEDY_1 $folder/log/1x/greedy/profiler \
        CLUSTER_10 $folder/log/10x/cluster/profiler BLOOM_10 $folder/log/10x/bloom/profiler GREEDY_SIMPLE_10 $folder/log/10x/greedy_simple/profiler GREEDY_10 $folder/log/10x/greedy/profiler \
        CLUSTER_50 $folder/log/50x/cluster/profiler BLOOM_50 $folder/log/50x/bloom/profiler GREEDY_SIMPLE_50 $folder/log/50x/greedy_simple/profiler GREEDY_50 $folder/log/50x/greedy/profiler \
        CLUSTER_200 $folder/log/200x/cluster/profiler BLOOM_200 $folder/log/200x/bloom/profiler GREEDY_SIMPLE_200 $folder/log/200x/greedy_simple/profiler GREEDY_200 $folder/log/200x/greedy/profiler

mv pruning.csv $folder/processed/
mv query_time.csv $folder/processed/
mv stats_eval_time.csv $folder/processed/
mv size.csv $folder/processed/
# mv ingestion.csv $folder/processed/