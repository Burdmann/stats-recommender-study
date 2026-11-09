# ./run_experiments.sh sql_queries_yellow_tripdata_2024.sql experiment_results/taxi_experiment/ 1171504 1 15 100
# ./run_experiments.sh sql_queries_chicago_taxi.sql experiment_results/chicago_taxi 1000000 1 20 100
# ./run_experiments.sh sql_queries_flights.sql experiment_results/flights 1000000 1 29 100
# ./run_experiments.sh sql_queries_ssb_flat.sql experiment_results/ssb_flat 1000000 1 25 100

./run_experiments.sh sql_queries_flights.sql experiment_results/flights 392272 1 26 100
./run_experiments.sh sql_queries_flights_line_plot.sql experiment_results/flights_line_plot 392272 1 32 100
./run_experiments.sh sql_queries_github.sql experiment_results/github 11596739 1 30 100
./run_experiments.sh sql_queries_github_line_plot.sql experiment_results/github_line_plot 11596739 1 20 100