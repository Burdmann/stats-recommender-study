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
	NONE = 0,
	MIN_MAX = 1,
	CLUSTER_SMALL = 2,
	CLUSTER_MEDIUM = 3,
	CLUSTER_LARGE = 4,
	BLOOM_SMALL = 5,
	BLOOM_MEDIUM = 6,
	BLOOM_LARGE = 7,
	DICTIONARY = 8
};

class StatisticsSet {
public:
	static STATISTIC_TYPE default_type;
	template <class T>
	static inline AdditionalStats<T> *construct(std::vector<T> &data, STATISTIC_TYPE type) {
		switch (type) {
		case STATISTIC_TYPE::NONE:
			return new EmptyAdditionalStats<T>(data);
		case STATISTIC_TYPE::MIN_MAX:
			return new ClusterAdditionalStats<T>(data, 1);
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
