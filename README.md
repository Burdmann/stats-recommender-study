# ...

This is the source code used in the paper "...", submitted to ...

## Structure

This repositry contains 2 folders, `duckdb/` and `experiments/`.

The `duckdb/` folder contains the full source code of duckdb, along with changes made to facilitate the use of richer column statistics alongside min/max indexes.

The `experiments/` folder contains bash scripts and SQL files used to run experiments to evaluate pruning and query-time performance of DuckDB using the additional statistics.

## Additions made to DuckDB

For a full list of changes, see the bottom of the page

Two further statistics come close to the space lower bound of approximate membership (see [pruning_codec.hpp](./duckdb/src/include/duckdb/storage/statistics/additional/pruning_codec.hpp)), using the same space as the Bloom filters:

- `NullspaceAdditionalStats` (point queries): keys are hashed into buckets; per bucket it stores nonzero vectors `v` with `a(x) . v = 0` over GF(2)/GF(3) for every key `x`, and prunes a query `q` if `a(q) . v != 0`. A non-member passes a GF(2) layer with probability 1/2 at 1 bit per key, and a GF(3) layer with probability 1/3 at log2(3) bits per key.
- `OccupancyAdditionalStats` (range and point queries): the occupied cells of an equal-width grid over the values, Golomb-coded, with the finest cell width that fits. A range is pruned if all cells it touches are empty.

`experiments/run_experiments_optimal.sh` measures their pruning on the unsorted micro benchmark next to min/max and Bloom filters.

`<Level>` should be one of `0`,`1`, or `2`, where 0 corresponds to the smallest metadata-size (10x the of min/max), 1 is the middle configuration at 100x the size, and 2 is the largest configuration, iwth 1000x the metadata-size used by min/max indexes.

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
