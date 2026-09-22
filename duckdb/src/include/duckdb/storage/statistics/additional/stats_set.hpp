//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/storage/statistics/additional/stats_set.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/storage/statistics/additional/additional_stats.hpp"
#include "duckdb/storage/statistics/additional/empty_additional_stats.hpp"
#include "duckdb/storage/statistics/additional/cluster_additional_stats.hpp"
#include "duckdb/storage/statistics/additional/bloom_additional_stats.hpp"
#include "duckdb/storage/statistics/additional/always_prune_additional_stats.hpp"
#include "duckdb/storage/statistics/additional/dictionary_additional_stats.hpp"

namespace duckdb {

enum class STATISTIC_TYPE : uint8_t {
	MIN_MAX = 0,
	CLUSTER_SMALL = 1,
	CLUSTER_MEDIUM = 2,
	CLUSTER_LARGE = 3,
	BLOOM_SMALL = 4,
	BLOOM_MEDIUM = 5,
	BLOOM_LARGE = 6,
	DICTIONARY = 7
};

class StatisticsSet {
public:
	template <class T>
	static inline AdditionalStats<T> *construct(std::vector<T> &data, STATISTIC_TYPE type) {
		switch (type) {
		case STATISTIC_TYPE::MIN_MAX:
			return new EmptyAdditionalStats<T>(data);
		case STATISTIC_TYPE::CLUSTER_SMALL:
			return new ClusterAdditionalStats<T>(data, 10);
		case STATISTIC_TYPE::CLUSTER_MEDIUM:
			return new ClusterAdditionalStats<T>(data, 50);
		case STATISTIC_TYPE::CLUSTER_LARGE:
			return new ClusterAdditionalStats<T>(data, 200);
		case STATISTIC_TYPE::BLOOM_SMALL:
			return new BloomAdditionalStats<T>(data, 1, 20, 1);
		case STATISTIC_TYPE::BLOOM_MEDIUM:
			return new BloomAdditionalStats<T>(data, 1, 100, 1);
		case STATISTIC_TYPE::BLOOM_LARGE:
			return new BloomAdditionalStats<T>(data, 2, 400, 1);
		case STATISTIC_TYPE::DICTIONARY:
			return new DictionaryAdditionalStats<T>(data, 2000);
		default:
			return NULL;
		}
	}
};
} // namespace duckdb
