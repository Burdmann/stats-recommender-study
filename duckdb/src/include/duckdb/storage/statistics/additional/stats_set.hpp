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

enum class STATISTIC_TYPE : int8_t {
	ERROR = -1,
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
		AdditionalStats<T> *res;
		switch (type) {
		case STATISTIC_TYPE::ERROR:
			throw std::invalid_argument("Cannot initialise statistics of type -1");
		case STATISTIC_TYPE::NONE:
			res = new EmptyAdditionalStats<T>(data);
			break;
		case STATISTIC_TYPE::MIN_MAX:
			res = new ClusterAdditionalStats<T, 1>(data);
			break;
		case STATISTIC_TYPE::CLUSTER_SMALL:
			res = new ClusterAdditionalStats<T, 10>(data);
			break;
		case STATISTIC_TYPE::CLUSTER_MEDIUM:
			res = new ClusterAdditionalStats<T, 50>(data);
			break;
		case STATISTIC_TYPE::CLUSTER_LARGE:
			res = new ClusterAdditionalStats<T, 200>(data);
			break;
		case STATISTIC_TYPE::BLOOM_SMALL:
			res = new BloomAdditionalStats<T, 20, 1>(data, 1);
			break;
		case STATISTIC_TYPE::BLOOM_MEDIUM:
			res = new BloomAdditionalStats<T, 100, 1>(data, 1);
			break;
		case STATISTIC_TYPE::BLOOM_LARGE:
			res = new BloomAdditionalStats<T, 400, 1>(data, 2);
			break;
		case STATISTIC_TYPE::DICTIONARY:
			res = new DictionaryAdditionalStats<T, 2000>(data);
			break;
		default:
			return NULL;
		}
		res->type = type;
		return res;
	}
};
} // namespace duckdb
