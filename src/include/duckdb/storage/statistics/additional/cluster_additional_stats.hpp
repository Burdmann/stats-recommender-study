//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/storage/statistics/additional/cluster_additional_stats.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include <functional>
#include <iostream>
#include "duckdb/storage/statistics/additional/additional_stats.hpp"
#include "duckdb/common/enums/filter_propagate_result.hpp"
#include "duckdb/common/operator/comparison_operators.hpp"
#include "duckdb/storage/statistics/base_statistics.hpp"
#include "duckdb/common/enums/expression_type.hpp"

namespace duckdb {

// constexpr static uint32_t MAX_NUMBER_OF_CLUSTERS = 100;
constexpr static uint32_t MAX_NUMBER_OF_CLUSTERS = LARGE_ADDITIONAL_STATS ? 1000 : 100;
#define CLUSTER_MAX_STRING_MINMAX_SIZE 8

template <class T>
class ClusterAdditionalStats : public AdditionalStats<T> {
private:
	unsigned int cluster_count = 0;
	std::vector<T> min_values;
	std::vector<T> max_values;
	static bool ConstantExactRange(T min, T max, T constant) {
		return Equals::Operation(constant, min) && Equals::Operation(constant, max);
	}
	static bool ConstantValueInRange(T min, T max, T constant) {
		return !(LessThan::Operation(constant, min) || GreaterThan::Operation(constant, max));
	}

public:
	static inline const char *GetStaticName() {
		return "cluster";
	}
	inline ClusterAdditionalStats(std::vector<T> &data) {
		this->name = GetStaticName();
		this->Initialise = &Initialise_implementation;
		this->Query = &Query_implementation;
		this->QueryRange = &QueryRange_implementation;
		this->Size = &Size_implementation;
		this->Serialise = &Serialise_implementation;
		this->Deserialise = &Deserialise_implementation;
		this->Initialise(data, this);
		this->type = ADDITIONAL_STATS_TYPE::CLUSTER;
	}

	inline static void Initialise_implementation(std::vector<T> &data, AdditionalStats<T> *stats) {
		ClusterAdditionalStats<T> *nstats = static_cast<ClusterAdditionalStats<T> *>(stats);
		nstats->cluster_count = 0;

		int size = data.size();
		if (size == 0)
			return;

		// sort the data
		std::sort(data.begin(), data.end());

		// compute gap sizes
		std::vector<std::pair<T, int>> gaps; // first element of each pair stores the gap, second is the index
		for (int i = 0; i < size - 1; i++) {
			gaps.push_back({data[i + 1] - data[i], i});
		}

		// select the k largest gaps
		T zero = (T)0;
		std::vector<int> idxs;
		std::sort(gaps.begin(), gaps.end(), std::greater<std::pair<T, int>>());

		for (int i = 0; i < std::min((unsigned long)gaps.size(), (unsigned long)(MAX_NUMBER_OF_CLUSTERS - 1)); i++) {
			if (gaps[i].first == zero)
				break;
			idxs.push_back(gaps[i].second);
		}

		// save min/max ranges defined by those gaps in nstats
		std::sort(idxs.begin(), idxs.end());
		T start = data[0];
		for (int idx : idxs) {
			nstats->min_values.push_back(start);
			nstats->max_values.push_back(data[idx]);
			start = data[idx + 1];
			nstats->cluster_count++;
		}

		nstats->min_values.push_back(start);
		nstats->max_values.push_back(data.back());
		nstats->cluster_count++;
	}

	inline static idx_t FindLastIndexBeforePoint_Binary(std::vector<T> &min_values, const T &constant) {
		idx_t lo = 0;
		idx_t hi = min_values.size() - 1;
		idx_t mid;
		while (lo < hi) {
			mid = (lo + hi + 1) / 2;
			if (min_values[mid] > constant) {
				hi = mid - 1;
			} else if (min_values[mid] < constant) {
				lo = mid;
			} else {
				// min_values[mid] == constant
				return mid;
			}
		}
		if (min_values[mid] > constant)
			return -1;
		return mid;
	}

	inline static idx_t FindLastIndexBeforePoint_Linear(std::vector<T> &min_values, const T &constant) {
		for (int i = 0; i < min_values.size(); i++) {
			if (min_values[i] > constant) {
				return i - 1;
			}
		}
		return min_values.size() - 1;
	}

	inline static idx_t FindLastIndexBeforePoint(std::vector<T> &min_values, const T &constant) {
		return FindLastIndexBeforePoint_Linear(min_values, constant);
	}

	inline static FilterPropagateResult Query_Equal(ClusterAdditionalStats<T> *nstats, const T &constant) {
		int idx = FindLastIndexBeforePoint(nstats->min_values, constant);
		if (idx == -1) {
			return FilterPropagateResult::FILTER_ALWAYS_FALSE;
		} else if (nstats->max_values[idx] >= constant) {
			return FilterPropagateResult::NO_PRUNING_POSSIBLE;
		} else {
			return FilterPropagateResult::FILTER_ALWAYS_FALSE;
		}
	}

	inline static FilterPropagateResult Query_implementation(AdditionalStats<T> *stats, ExpressionType &comparison_type,
	                                                         const T &constant) {
		ClusterAdditionalStats<T> *nstats = (ClusterAdditionalStats<T> *)stats;
		// printf("nstats->cluster_count: %d\n", nstats->cluster_count);
		if (nstats->cluster_count == 0)
			return FilterPropagateResult::NO_PRUNING_POSSIBLE;
		// printf("comparison_type: %s\n", ExpressionTypeToString(comparison_type).c_str());
		// std::cout << "constant: " << (int64_t)constant << "\n";
		// for (int i = 0; i < nstats->cluster_count; i++) {
		// 	std::cout << "cluster: " << (int64_t)nstats->min_values[i] << " -> " << (int64_t)nstats->max_values[i]
		// 	          << "\n";
		// }

		switch (comparison_type) {
		case ExpressionType::COMPARE_EQUAL:
		case ExpressionType::COMPARE_NOT_DISTINCT_FROM:
			return Query_Equal(nstats, constant);
		case ExpressionType::COMPARE_NOTEQUAL:
		case ExpressionType::COMPARE_DISTINCT_FROM:
		case ExpressionType::COMPARE_GREATERTHANOREQUALTO:
		case ExpressionType::COMPARE_GREATERTHAN:
		case ExpressionType::COMPARE_LESSTHANOREQUALTO:
		case ExpressionType::COMPARE_LESSTHAN:
			return FilterPropagateResult::NO_PRUNING_POSSIBLE;
		default:
			throw InternalException("Expression type in zonemap check not implemented");
		}
	}
	inline static FilterPropagateResult QueryRange_implementation(AdditionalStats<T> *stats, const T &start,
	                                                              const T &end) {
		ClusterAdditionalStats<T> *nstats = (ClusterAdditionalStats<T> *)stats;
		if (nstats->cluster_count == 0)
			return FilterPropagateResult::NO_PRUNING_POSSIBLE;
		int idx = FindLastIndexBeforePoint(nstats->min_values, start);
		if (idx == -1) {
			if (nstats->min_values[0] > end) {
				return FilterPropagateResult::FILTER_ALWAYS_FALSE;
			} else {
				return FilterPropagateResult::NO_PRUNING_POSSIBLE;
			}
		} else if (nstats->max_values[idx] >= start ||
		           (idx < nstats->cluster_count - 1 && nstats->min_values[idx + 1] <= end)) {
			return FilterPropagateResult::NO_PRUNING_POSSIBLE;
		} else {
			return FilterPropagateResult::FILTER_ALWAYS_FALSE;
		}
	}
	inline static size_t Size_implementation(AdditionalStats<T> *stats) {
		ClusterAdditionalStats<T> *nstats = (ClusterAdditionalStats<T> *)stats;
		return 2 * sizeof(T) * nstats->cluster_count + sizeof(*nstats);
	}
	inline static void Serialise_implementation(AdditionalStats<T> *stats, Serializer &serializer) {
	}
	inline static void Deserialise_implementation(AdditionalStats<T> *stats, Deserializer &deserializer) {
	}
};

struct data_array {
public:
	duckdb::data_t data[CLUSTER_MAX_STRING_MINMAX_SIZE];
};

template <>
class ClusterAdditionalStats<std::string> : public AdditionalStats<std::string> {
private:
	unsigned int cluster_count = 0;
	std::vector<data_array> min_values;
	std::vector<data_array> max_values;
	static bool ConstantExactRange(std::string min, std::string max, std::string constant) {
		return Equals::Operation(constant, min) && Equals::Operation(constant, max);
	}
	static bool ConstantValueInRange(std::string min, std::string max, std::string constant) {
		return !(LessThan::Operation(constant, min) || GreaterThan::Operation(constant, max));
	}
	static inline unsigned long long StringToLong(const std::string str) {
		unsigned long long res = 0;
		const char *data = str.data();
		for (int i = 0; i < str.size(); i++) {
			res <<= 7;
			res += data[i];
		}
		return res;
	}

	static int StringValueComparison(const_data_ptr_t data, idx_t len, const_data_ptr_t comparison) {
		for (idx_t i = 0; i < len; i++) {
			if (data[i] < comparison[i]) {
				return -1;
			} else if (data[i] > comparison[i]) {
				return 1;
			}
		}
		return 0;
	}

	static void ConstructValue(const_data_ptr_t data, idx_t size, data_t *target) {
		idx_t value_size = size > CLUSTER_MAX_STRING_MINMAX_SIZE ? CLUSTER_MAX_STRING_MINMAX_SIZE : size;
		memcpy(target, data, value_size);
		for (idx_t i = value_size; i < CLUSTER_MAX_STRING_MINMAX_SIZE; i++) {
			target[i] = '\0';
		}
	}

public:
	static inline const char *GetStaticName() {
		return "cluster";
	}
	inline ClusterAdditionalStats(std::vector<std::string> &data) {
		this->name = GetStaticName();
		this->Initialise = &Initialise_implementation;
		this->Query = &Query_implementation;
		this->QueryRange = &QueryRange_implementation;
		this->Size = &Size_implementation;
		this->Serialise = &Serialise_implementation;
		this->Deserialise = &Deserialise_implementation;
		this->Initialise(data, this);
	}

	inline static void Initialise_implementation(std::vector<std::string> &data, AdditionalStats<std::string> *stats) {
		ClusterAdditionalStats<std::string> *nstats = (ClusterAdditionalStats<std::string> *)stats;
		nstats->cluster_count = 0;

		int size = data.size();
		if (size == 0)
			return;

		// sort the data
		std::sort(data.begin(), data.end());

		// compute gap sizes
		std::vector<std::pair<unsigned long long, int>>
		    gaps; // first element of each pair stores the gap, second is the index
		for (int i = 0; i < size - 1; i++) {
			gaps.push_back({StringToLong(data[i + 1]) - StringToLong(data[i]), i});
		}

		// select the k largest gaps
		std::vector<int> idxs;
		std::sort(gaps.begin(), gaps.end(), std::greater<std::pair<unsigned long long, int>>());

		for (int i = 0; i < std::min((unsigned long)gaps.size(), (unsigned long)(MAX_NUMBER_OF_CLUSTERS - 1)); i++) {
			if (gaps[i].first == 0)
				break;
			idxs.push_back(gaps[i].second);
		}

		// save min/max ranges defined by those gaps in nstats
		std::sort(idxs.begin(), idxs.end());
		data_array start;
		data_array next;
		ConstructValue(const_data_ptr_cast(data[0].data()), data[0].size(), start.data);
		for (int idx : idxs) {
			nstats->min_values.push_back(start);
			ConstructValue(const_data_ptr_cast(data[idx].data()), data[idx].size(), next.data);
			nstats->max_values.push_back(next);
			ConstructValue(const_data_ptr_cast(data[idx + 1].data()), data[idx + 1].size(), start.data);
			nstats->cluster_count++;
		}

		nstats->min_values.push_back(start);
		ConstructValue(const_data_ptr_cast(data.back().data()), data.back().size(), next.data);
		nstats->max_values.push_back(next);
		nstats->cluster_count++;
	}

	inline static FilterPropagateResult Query_inner(duckdb::data_t min_value[CLUSTER_MAX_STRING_MINMAX_SIZE],
	                                                duckdb::data_t max_value[CLUSTER_MAX_STRING_MINMAX_SIZE],
	                                                ExpressionType &comparison_type, const std::string &constant) {
		auto data = const_data_ptr_cast(constant.data());
		idx_t size = constant.size();

		int min_comp = StringValueComparison(data, MinValue((idx_t)CLUSTER_MAX_STRING_MINMAX_SIZE, size),
		                                     (duckdb::data_t *)min_value);
		int max_comp = StringValueComparison(data, MinValue((idx_t)CLUSTER_MAX_STRING_MINMAX_SIZE, size),
		                                     (duckdb::data_t *)max_value);
		switch (comparison_type) {
		case ExpressionType::COMPARE_EQUAL:
		case ExpressionType::COMPARE_NOT_DISTINCT_FROM:
			if (min_comp >= 0 && max_comp <= 0) {
				return FilterPropagateResult::NO_PRUNING_POSSIBLE;
			} else {
				return FilterPropagateResult::FILTER_ALWAYS_FALSE;
			}
		case ExpressionType::COMPARE_NOTEQUAL:
		case ExpressionType::COMPARE_DISTINCT_FROM:
		case ExpressionType::COMPARE_GREATERTHANOREQUALTO:
		case ExpressionType::COMPARE_GREATERTHAN:
		case ExpressionType::COMPARE_LESSTHAN:
		case ExpressionType::COMPARE_LESSTHANOREQUALTO:
			// if the regular string stats could not rule out this partition, neither can clusters
			return FilterPropagateResult::NO_PRUNING_POSSIBLE;
		default:
			throw InternalException("Expression type not implemented for string statistics zone map");
		}
	}
	inline static FilterPropagateResult Query_implementation(AdditionalStats<std::string> *stats,
	                                                         ExpressionType &comparison_type,
	                                                         const std::string &constant) {
		ClusterAdditionalStats<std::string> *nstats = (ClusterAdditionalStats<std::string> *)stats;
		// printf("nstats->cluster_count: %d\n", nstats->cluster_count);
		if (nstats->cluster_count == 0)
			return FilterPropagateResult::NO_PRUNING_POSSIBLE;
		// printf("comparison_type: %s\n", ExpressionTypeToString(comparison_type).c_str());
		// std::cout << "constant: " << constant << "\n";
		// for (int i = 0; i < nstats->cluster_count; i++) {
		// 	std::cout << "cluster: ";
		// 	for (int j = 0; j < CLUSTER_MAX_STRING_MINMAX_SIZE && nstats->min_values[i].data[j]; j++) {
		// 		std::cout << (char)nstats->min_values[i].data[j];
		// 	}
		// 	std::cout << " -> ";
		// 	for (int j = 0; j < CLUSTER_MAX_STRING_MINMAX_SIZE && nstats->max_values[i].data[j]; j++) {
		// 		std::cout << (char)nstats->max_values[i].data[j];
		// 	}
		// 	std::cout << "\n";
		// }
		for (int i = 0; i < nstats->min_values.size(); i++) {
			FilterPropagateResult result =
			    Query_inner(nstats->min_values[i].data, nstats->max_values[i].data, comparison_type, constant);
			if (result == FilterPropagateResult::FILTER_ALWAYS_TRUE) {
				return FilterPropagateResult::NO_PRUNING_POSSIBLE;
			} else if (result == FilterPropagateResult::NO_PRUNING_POSSIBLE)
				return FilterPropagateResult::NO_PRUNING_POSSIBLE;
		}
		return FilterPropagateResult::FILTER_ALWAYS_FALSE;
	}

	inline static FilterPropagateResult QueryRange_implementation(AdditionalStats<std::string> *stats,
	                                                              const std::string &start, const std::string &end) {
		ClusterAdditionalStats<std::string> *nstats = (ClusterAdditionalStats<std::string> *)stats;
		if (nstats->cluster_count == 0)
			return FilterPropagateResult::NO_PRUNING_POSSIBLE;
		// TODO: implement
		return FilterPropagateResult::NO_PRUNING_POSSIBLE;
	}

	inline static size_t Size_implementation(AdditionalStats<std::string> *stats) {
		ClusterAdditionalStats<std::string> *nstats = (ClusterAdditionalStats<std::string> *)stats;
		return 2 * (sizeof(std::string) + CLUSTER_MAX_STRING_MINMAX_SIZE) * nstats->cluster_count + sizeof(*nstats);
	}
	inline static void Serialise_implementation(AdditionalStats<std::string> *stats, Serializer &serializer) {
	}
	inline static void Deserialise_implementation(AdditionalStats<std::string> *stats, Deserializer &deserializer) {
	}
};

} // namespace duckdb