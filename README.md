# Benchmarking Column Statistics for Analytical Query Pruning

This is the source code used in the paper "Benchmarking Column Statistics for Analytical Query Pruning", submitted to the workshop DBTest 2026

## Structure

This repositry contains 2 folders, `duckdb/` and `experiments/`.

The `duckdb/` folder contains the full source code of duckdb, along with changes made to facilitate the use of richer column statistics alongside min/max indexes.

The `experiments/` folder contains bash scripts ans SQL files used to run experiments to evaluate pruning and query-time performance of DuckDB using the additional statistics.

## Additions made to DuckDB

For a full list of changes, see the bottom of the page

We created the class `AdditionalStats`, with different types of addition stats (e.g. cluster, bloom filter, dictionary) being subclasses of this class.

`AdditionalStats` objects are attached to partitions through `BaseStats` objects, which contains the default min/max index.

`AdditionalStats` objects must have an initialisation method, that takes a vector of data and uses it to construct itself.
Additionally, they must also have a query method, where a value and a comparison type can be given to it, and it can answer whether or not the corresponding partition can be pruned based on that.
A few other methods must also be present, including a range query method and one for measuring size of the statistics object.

`AdditionalStats` is defined in [additional_stats.hpp](./duckdb/src/include/duckdb/storage/statistics/additional/additional_stats.hpp), with concrete implementations present in the same folders using only headers.

To use these new statistics, we also introduced some logic in [constant_filter.cpp](./duckdb/src/planner/filter/constant_filter.cpp), to check the query method of the `AdditionalStats` object when min/max alone is not enough to determine that the pertition may be pruned.

The remaining changes are mainly just there to falicitate the creation and use of these `AdditionalStats`.

## Compiling Source-code

In the root of the repository, a bash script named `compile_all.sh` exists, running this will create 10 seperate binaries of DuckDB each with a different configuration of additional statistics.
Compiling a single binary can be done by navigating to the duckdb folder and calling

`make STATS=<AdditionalStats> ADDITIONAL_STATS_SCALE_LEVEL=<Level>`

Where `<AdditionalStats>` is one of `EmptyAdditionalStats`, `ClusterAdditionalStats`, `BloomAdditionalStats`, or `DictionaryAdditionalStats` for regular min/max, min/max clusters, bloom filters or dictionaries respectively.

`<Level>` should be one of `0`,`1`, or `2`, where 0 corresponds to the smallest metadata-size (10x the of min/max), 1 is the middle configuration at 100x the size, and 2 is the largest configuration, iwth 1000x the metadata-size used by min/max indexes.

## Generating Datasets

To generate datasets, the bash script `generate_datasets.sh` can be used. This script first generates an unsorted dataset with 1,000,000,000 rows and 2 columns, each of which has type unsigned 64-bit int. It then generates the sorted dataset, which is sorted by the first column, using the unsorted one. After that, it uses the sorted dataset to generate the outlier dataset. And finally, it uses the sorted dataset to generate 4 datasets with varying cardinalities of the second column (cardinalities of 100, 1,000, 10,000, 100,000).

## Running Experiments

To run experiments, the bash scripts in the `experiments/` folder can be used. The scripts suffixed with `_performance` will measure query running time, whereas the ones without this suffix measures pruning efficiency.

To see which SQL queries are used, navigate to the folder `experiments/sql/`

## Full list of changed files

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
- [always_prune_additional_stats.hpp](./duckdb/src/include/duckdb/storage/statistics/additional/always_prune_additional_stats.hpp)
- [bloom_additional_stats.hpp](./duckdb/src/include/duckdb/storage/statistics/additional/bloom_additional_stats.hpp)
- [cluster_additional_stats.hpp](./duckdb/src/include/duckdb/storage/statistics/additional/cluster_additional_stats.hpp)
- [dictionary_additional_stats.hpp](./duckdb/src/include/duckdb/storage/statistics/additional/dictionary_additional_stats.hpp)
- [empty_additional_stats.hpp](./duckdb/src/include/duckdb/storage/statistics/additional/empty_additional_stats.hpp)
- [util.hpp](./duckdb/src/include/duckdb/util/util.hpp)
- [util.cpp](./duckdb/src/util/util.cpp)
