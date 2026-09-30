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
#include <functional>

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

struct StatisticsSetKeyHash {
	size_t operator()(const std::tuple<std::string, unsigned int, unsigned int> &key) const {
		auto hash = std::hash<std::string>()(std::get<0>(key));
		hash = hash * 31 + std::hash<int>()(std::get<1>(key));
		return hash * 31 + std::hash<int>()(std::get<2>(key));
	}
};

class StatisticsSet {
public:
	static STATISTIC_TYPE default_type;
	static std::unordered_map<std::tuple<std::string, unsigned int, unsigned int>, STATISTIC_TYPE, StatisticsSetKeyHash>
	    mapping;
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
			res = new DictionaryAdditionalStats<T, 300>(data);
			break;
		default:
			return NULL;
		}
		res->type = type;
		return res;
	}

	template <class T>
	static inline AdditionalStats<T> *construct(std::vector<T> &data, std::string table_name, unsigned int rowgroup,
	                                            unsigned int column) {
		auto ptr = mapping.find(std::make_tuple(table_name, rowgroup, column));
		if (ptr == mapping.end())
			return construct(data, default_type);
		else
			return construct(data, ptr->second);
	}
};
} // namespace duckdb
