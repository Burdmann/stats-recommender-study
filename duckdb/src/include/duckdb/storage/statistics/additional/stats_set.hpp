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
#include "duckdb/storage/statistics/additional/dictionary_additional_stats.hpp"
#include <functional>

namespace duckdb {

template <class T>
using NONE = EmptyAdditionalStats<T>;
template <class T>
using MIN_MAX = ClusterAdditionalStats<T, 1>;
template <class T>
using CLUSTER_SMALL = ClusterAdditionalStats<T, 10>;
template <class T>
using CLUSTER_MEDIUM = ClusterAdditionalStats<T, 50>;
template <class T>
using CLUSTER_LARGE = ClusterAdditionalStats<T, 200>;
template <class T>
using BLOOM_SMALL = BloomAdditionalStats<T, 20, 1, 1>;
template <class T>
using BLOOM_MEDIUM = BloomAdditionalStats<T, 100, 1, 1>;
template <class T>
using BLOOM_LARGE = BloomAdditionalStats<T, 400, 1, 2>;
template <class T>
using DICTIONARY = DictionaryAdditionalStats<T, 300>;

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
			res = new NONE<T>(data);
			break;
		case STATISTIC_TYPE::MIN_MAX:
			res = new MIN_MAX<T>(data);
			break;
		case STATISTIC_TYPE::CLUSTER_SMALL:
			res = new CLUSTER_SMALL<T>(data);
			break;
		case STATISTIC_TYPE::CLUSTER_MEDIUM:
			res = new CLUSTER_MEDIUM<T>(data);
			break;
		case STATISTIC_TYPE::CLUSTER_LARGE:
			res = new CLUSTER_LARGE<T>(data);
			break;
		case STATISTIC_TYPE::BLOOM_SMALL:
			res = new BLOOM_SMALL<T>(data);
			break;
		case STATISTIC_TYPE::BLOOM_MEDIUM:
			res = new BLOOM_MEDIUM<T>(data);
			break;
		case STATISTIC_TYPE::BLOOM_LARGE:
			res = new BLOOM_LARGE<T>(data);
			break;
		case STATISTIC_TYPE::DICTIONARY:
			res = new DICTIONARY<T>(data);
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

	template <class T>
	static inline FilterPropagateResult Query(AdditionalStats<T> *stats, ExpressionType comparison_type,
	                                          const T &constant) {
		switch (stats->type) {
		case STATISTIC_TYPE::ERROR:
			throw std::invalid_argument("Cannot initialise statistics of type -1");
		case STATISTIC_TYPE::NONE:
			return NONE<T>::Query(stats, comparison_type, constant);
		case STATISTIC_TYPE::MIN_MAX:
			return MIN_MAX<T>::Query(stats, comparison_type, constant);
		case STATISTIC_TYPE::CLUSTER_SMALL:
			return CLUSTER_SMALL<T>::Query(stats, comparison_type, constant);
		case STATISTIC_TYPE::CLUSTER_MEDIUM:
			return CLUSTER_MEDIUM<T>::Query(stats, comparison_type, constant);
		case STATISTIC_TYPE::CLUSTER_LARGE:
			return CLUSTER_LARGE<T>::Query(stats, comparison_type, constant);
		case STATISTIC_TYPE::BLOOM_SMALL:
			return BLOOM_SMALL<T>::Query(stats, comparison_type, constant);
		case STATISTIC_TYPE::BLOOM_MEDIUM:
			return BLOOM_MEDIUM<T>::Query(stats, comparison_type, constant);
		case STATISTIC_TYPE::BLOOM_LARGE:
			return BLOOM_LARGE<T>::Query(stats, comparison_type, constant);
		case STATISTIC_TYPE::DICTIONARY:
			return DICTIONARY<T>::Query(stats, comparison_type, constant);
		default:
			return FilterPropagateResult::NO_PRUNING_POSSIBLE;
		}
	}
	template <class T>
	static inline FilterPropagateResult QueryRange(AdditionalStats<T> *stats, const T &start, const T &end) {
		switch (stats->type) {
		case STATISTIC_TYPE::ERROR:
			throw std::invalid_argument("Cannot initialise statistics of type -1");
		case STATISTIC_TYPE::NONE:
			return NONE<T>::QueryRange(stats, start, end);
		case STATISTIC_TYPE::MIN_MAX:
			return MIN_MAX<T>::QueryRange(stats, start, end);
		case STATISTIC_TYPE::CLUSTER_SMALL:
			return CLUSTER_SMALL<T>::QueryRange(stats, start, end);
		case STATISTIC_TYPE::CLUSTER_MEDIUM:
			return CLUSTER_MEDIUM<T>::QueryRange(stats, start, end);
		case STATISTIC_TYPE::CLUSTER_LARGE:
			return CLUSTER_LARGE<T>::QueryRange(stats, start, end);
		case STATISTIC_TYPE::BLOOM_SMALL:
			return BLOOM_SMALL<T>::QueryRange(stats, start, end);
		case STATISTIC_TYPE::BLOOM_MEDIUM:
			return BLOOM_MEDIUM<T>::QueryRange(stats, start, end);
		case STATISTIC_TYPE::BLOOM_LARGE:
			return BLOOM_LARGE<T>::QueryRange(stats, start, end);
		case STATISTIC_TYPE::DICTIONARY:
			return DICTIONARY<T>::QueryRange(stats, start, end);
		default:
			return FilterPropagateResult::NO_PRUNING_POSSIBLE;
		}
	}
	template <class T>
	static inline idx_t Size(AdditionalStats<T> *stats) {
		switch (stats->type) {
		case STATISTIC_TYPE::ERROR:
			throw std::invalid_argument("Cannot initialise statistics of type -1");
		case STATISTIC_TYPE::NONE:
			return NONE<T>::Size(stats);
		case STATISTIC_TYPE::MIN_MAX:
			return MIN_MAX<T>::Size(stats);
		case STATISTIC_TYPE::CLUSTER_SMALL:
			return CLUSTER_SMALL<T>::Size(stats);
		case STATISTIC_TYPE::CLUSTER_MEDIUM:
			return CLUSTER_MEDIUM<T>::Size(stats);
		case STATISTIC_TYPE::CLUSTER_LARGE:
			return CLUSTER_LARGE<T>::Size(stats);
		case STATISTIC_TYPE::BLOOM_SMALL:
			return BLOOM_SMALL<T>::Size(stats);
		case STATISTIC_TYPE::BLOOM_MEDIUM:
			return BLOOM_MEDIUM<T>::Size(stats);
		case STATISTIC_TYPE::BLOOM_LARGE:
			return BLOOM_LARGE<T>::Size(stats);
		case STATISTIC_TYPE::DICTIONARY:
			return DICTIONARY<T>::Size(stats);
		default:
			return 0;
		}
	}
	template <class T>
	static inline const char *Name(AdditionalStats<T> *stats) {
		switch (stats->type) {
		case STATISTIC_TYPE::ERROR:
			throw std::invalid_argument("Cannot initialise statistics of type -1");
		case STATISTIC_TYPE::NONE:
			return NONE<T>::GetStaticName();
		case STATISTIC_TYPE::MIN_MAX:
			return MIN_MAX<T>::GetStaticName();
		case STATISTIC_TYPE::CLUSTER_SMALL:
			return CLUSTER_SMALL<T>::GetStaticName();
		case STATISTIC_TYPE::CLUSTER_MEDIUM:
			return CLUSTER_MEDIUM<T>::GetStaticName();
		case STATISTIC_TYPE::CLUSTER_LARGE:
			return CLUSTER_LARGE<T>::GetStaticName();
		case STATISTIC_TYPE::BLOOM_SMALL:
			return BLOOM_SMALL<T>::GetStaticName();
		case STATISTIC_TYPE::BLOOM_MEDIUM:
			return BLOOM_MEDIUM<T>::GetStaticName();
		case STATISTIC_TYPE::BLOOM_LARGE:
			return BLOOM_LARGE<T>::GetStaticName();
		case STATISTIC_TYPE::DICTIONARY:
			return DICTIONARY<T>::GetStaticName();
		default:
			return "error";
		}
	}
};
} // namespace duckdb
