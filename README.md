# ...

This is the source code used in the paper "...", submitted to ...

## Structure

This repositry contains 2 folders, `duckdb/` and `experiments/`.

The `duckdb/` folder contains the full source code of duckdb, along with changes made to facilitate the use of richer column statistics alongside min/max indexes.

The `experiments/` folder contains bash scripts and SQL files used to run experiments to evaluate pruning and query-time performance of DuckDB using the additional statistics.

## Additions made to DuckDB

For a full list of changes, see the bottom of the page

## Compiling Source-code

Navigate to the duckdb folder and run `make`, the binary can subsequently be found as `duckdb/build/release/duckdb`

## Running the binary

...

<!-- ## Generating Datasets

To generate datasets, the bash script `generate_datasets.sh` can be used. This script first generates an unsorted dataset with 1,000,000,000 rows and 2 columns, each of which has type unsigned 64-bit int. It then generates the sorted dataset, which is sorted by the first column, using the unsorted one. After that, it uses the sorted dataset to generate the outlier dataset. And finally, it uses the sorted dataset to generate 4 datasets with varying cardinalities of the second column (cardinalities of 100, 1,000, 10,000, 100,000). -->

## Running Experiments

...

## Full list of changed files

TODO: expand list

Changes have been made to the following files:

- [Makefile](./duckdb/Makefile)
- [CMakeLists.txt](./duckdb/CMakeLists.txt)
- [physical_batch_insert.cpp](./duckdb/src/execution/operator/persistent/physical_batch_insert.cpp)
- [physical_insert.cpp](./duckdb/src/execution/operator/persistent/physical_insert.cpp)
- [physical_operator.cpp](./duckdb/src/execution/physical_operator.cpp)
- [physical_batch_insert.hpp](./duckdb/src/include/duckdb/execution/operator/persistent/physical_batch_insert.hpp)
- [physical_insert.hpp](./duckdb/src/include/duckdb/execution/operator/persistent/physical_insert.hpp)
- [physical_operator.hpp](./duckdb/src/include/duckdb/execution/physical_operator.hpp)
- [data_table.hpp](./duckdb/src/include/duckdb/storage/data_table.hpp)
- [base_statistics.hpp](./duckdb/src/include/duckdb/storage/statistics/base_statistics.hpp)
- [column_data.hpp](./duckdb/src/include/duckdb/storage/table/column_data.hpp)
- [row_group.hpp](./duckdb/src/include/duckdb/storage/table/row_group.hpp)
- [local_storage.hpp](./duckdb/src/include/duckdb/transaction/local_storage.hpp)
- [constant_filter.cpp](./duckdb/src/planner/filter/constant_filter.cpp)
- [data_table.cpp](./duckdb/src/storage/data_table.cpp)
- [local_storage.cpp](./duckdb/src/storage/local_storage.cpp)
- [base_statistics.cpp](./duckdb/src/storage/statistics/base_statistics.cpp)
- [column_data.cpp](./duckdb/src/storage/table/column_data.cpp)
- [row_group.cpp](./duckdb/src/storage/table/row_group.cpp)
- [standard_column_data.cpp](./duckdb/src/storage/table/standard_column_data.cpp)

The following files have been added:

- [additional_stats.hpp](./duckdb/src/include/duckdb/storage/statistics/additional/additional_stats.hpp)
- [additional_stats_util.hpp](./duckdb/src/include/duckdb/storage/statistics/additional/additional_stats_util.hpp)
- [bloom_additional_stats.hpp](./duckdb/src/include/duckdb/storage/statistics/additional/bloom_additional_stats.hpp)
- [cluster_additional_stats.hpp](./duckdb/src/include/duckdb/storage/statistics/additional/cluster_additional_stats.hpp)
- [dictionary_additional_stats.hpp](./duckdb/src/include/duckdb/storage/statistics/additional/dictionary_additional_stats.hpp)
- [empty_additional_stats.hpp](./duckdb/src/include/duckdb/storage/statistics/additional/empty_additional_stats.hpp)
- [util.hpp](./duckdb/src/include/duckdb/util/util.hpp)
- [util.cpp](./duckdb/src/util/util.cpp)
