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
#include <cmath>
#include "duckdb/storage/statistics/additional/additional_stats.hpp"
#include "duckdb/common/enums/filter_propagate_result.hpp"
#include "duckdb/common/operator/comparison_operators.hpp"
#include "duckdb/storage/statistics/base_statistics.hpp"
#include "duckdb/common/enums/expression_type.hpp"

namespace duckdb {

#define CLUSTER_MAX_STRING_MINMAX_SIZE 8

template <class T>
struct ClusterValueTraits {
	static bool IsNaN(T) {
		return false;
	}
	static bool IsInfinity(T) {
		return false;
	}
	static bool IsNegativeInfinity(T) {
		return false;
	}
};

template <>
struct ClusterValueTraits<float> {
	static bool IsNaN(float value) {
		return std::isnan(value);
	}
	static bool IsInfinity(float value) {
		return std::isinf(value);
	}
	static bool IsNegativeInfinity(float value) {
		return std::isinf(value) && value < 0;
	}
};

template <>
struct ClusterValueTraits<double> {
	static bool IsNaN(double value) {
		return std::isnan(value);
	}
	static bool IsInfinity(double value) {
		return std::isinf(value);
	}
	static bool IsNegativeInfinity(double value) {
		return std::isinf(value) && value < 0;
	}
};

template <class T, unsigned int N>
class ClusterAdditionalStats : public AdditionalStats<T> {
private:
	struct DuckDBLess {
		bool operator()(const T &left, const T &right) const {
			return GreaterThan::Operation(right, left);
		}
	};
	static const unsigned int MAX_CLUSTERS = N + 3;
	unsigned int cluster_count = 0;
	T min_values[MAX_CLUSTERS];
	T max_values[MAX_CLUSTERS];
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
	}

	inline static void Initialise_implementation(std::vector<T> &data, AdditionalStats<T> *stats) {
		ClusterAdditionalStats<T, N> *nstats = static_cast<ClusterAdditionalStats<T, N> *>(stats);
		nstats->cluster_count = 0;

		int size = data.size();
		if (size == 0)
			return;

		std::sort(data.begin(), data.end(), DuckDBLess());
		bool has_negative_infinity = false;
		bool has_positive_infinity = false;
		bool has_nan = false;
		T negative_infinity = T();
		T positive_infinity = T();
		T nan_value = T();
		for (idx_t i = 0; i < data.size(); i++) {
			T value = data[i];
			if (ClusterValueTraits<T>::IsNaN(value)) {
				has_nan = true;
				nan_value = value;
			} else if (ClusterValueTraits<T>::IsInfinity(value)) {
				if (ClusterValueTraits<T>::IsNegativeInfinity(value)) {
					has_negative_infinity = true;
					negative_infinity = value;
				} else {
					has_positive_infinity = true;
					positive_infinity = value;
				}
			}
		}
		const bool has_special_values = has_negative_infinity || has_positive_infinity || has_nan;
		std::vector<T> finite_data;
		if (has_special_values) {
			for (idx_t i = 0; i < data.size(); i++) {
				T value = data[i];
				if (!ClusterValueTraits<T>::IsNaN(value) && !ClusterValueTraits<T>::IsInfinity(value)) {
					finite_data.push_back(value);
				}
			}
		}
		auto &cluster_data = has_special_values ? finite_data : data;

		if (has_negative_infinity) {
			nstats->min_values[nstats->cluster_count] = negative_infinity;
			nstats->max_values[nstats->cluster_count++] = negative_infinity;
		}

		if (cluster_data.empty()) {
			if (has_positive_infinity) {
				nstats->min_values[nstats->cluster_count] = positive_infinity;
				nstats->max_values[nstats->cluster_count++] = positive_infinity;
			}
			if (has_nan) {
				nstats->min_values[nstats->cluster_count] = nan_value;
				nstats->max_values[nstats->cluster_count++] = nan_value;
			}
			return;
		}

		// compute gap sizes
		std::vector<std::pair<T, int>> gaps; // first element of each pair stores the gap, second is the index
		for (idx_t i = 0; i + 1 < cluster_data.size(); i++) {
			gaps.push_back({cluster_data[i + 1] - cluster_data[i], UnsafeNumericCast<int>(i)});
		}

		// select the k largest gaps
		T zero = (T)0;
		std::vector<int> idxs;
		std::sort(gaps.begin(), gaps.end(), std::greater<std::pair<T, int>>());

		for (int i = 0; i < std::min((unsigned long)gaps.size(), (unsigned long)(N - 1)); i++) {
			if (gaps[i].first == zero)
				break;
			idxs.push_back(gaps[i].second);
		}

		// save min/max ranges defined by those gaps in nstats
		std::sort(idxs.begin(), idxs.end());
		T start = cluster_data[0];
		for (int idx : idxs) {
			nstats->min_values[nstats->cluster_count] = start;
			nstats->max_values[nstats->cluster_count] = cluster_data[idx];
			start = cluster_data[idx + 1];
			nstats->cluster_count++;
		}

		nstats->min_values[nstats->cluster_count] = start;
		nstats->max_values[nstats->cluster_count++] = cluster_data.back();
		if (has_positive_infinity) {
			nstats->min_values[nstats->cluster_count] = positive_infinity;
			nstats->max_values[nstats->cluster_count++] = positive_infinity;
		}
		if (has_nan) {
			nstats->min_values[nstats->cluster_count] = nan_value;
			nstats->max_values[nstats->cluster_count++] = nan_value;
		}
	}

	inline static idx_t FindLastIndexBeforePoint_Binary(T *min_values, unsigned int len, const T &constant) {
		idx_t lo = 0;
		idx_t hi = len - 1;
		idx_t mid;
		while (lo < hi) {
			mid = (lo + hi + 1) / 2;
			if (GreaterThan::Operation(min_values[mid], constant)) {
				hi = mid - 1;
			} else if (LessThan::Operation(min_values[mid], constant)) {
				lo = mid;
			} else {
				// min_values[mid] == constant
				return mid;
			}
		}
		if (GreaterThan::Operation(min_values[mid], constant))
			return -1;
		return mid;
	}

	inline static idx_t FindLastIndexBeforePoint_Linear(T *min_values, unsigned int len, const T &constant) {
		for (int i = 0; i < len; i++) {
			if (GreaterThan::Operation(min_values[i], constant)) {
				return i - 1;
			}
		}
		return len - 1;
	}

	inline static idx_t FindLastIndexBeforePoint(T *min_values, unsigned int len, const T &constant) {
		return FindLastIndexBeforePoint_Linear(min_values, len, constant);
	}

	inline static FilterPropagateResult Query_Equal(ClusterAdditionalStats<T, N> *nstats, const T &constant) {
		int idx = FindLastIndexBeforePoint(nstats->min_values, nstats->cluster_count, constant);
		if (idx == -1) {
			return FilterPropagateResult::FILTER_ALWAYS_FALSE;
		} else if (GreaterThanEquals::Operation(nstats->max_values[idx], constant)) {
			return FilterPropagateResult::NO_PRUNING_POSSIBLE;
		} else {
			return FilterPropagateResult::FILTER_ALWAYS_FALSE;
		}
	}

	inline static FilterPropagateResult Query_implementation(AdditionalStats<T> *stats, ExpressionType &comparison_type,
	                                                         const T &constant) {
		ClusterAdditionalStats<T, N> *nstats = (ClusterAdditionalStats<T, N> *)stats;
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
			return (nstats->cluster_count == 1 && Equals::Operation(nstats->min_values[0], nstats->max_values[0]) &&
			        Equals::Operation(nstats->min_values[0], constant))
			           ? FilterPropagateResult::FILTER_ALWAYS_FALSE
					   : FilterPropagateResult::NO_PRUNING_POSSIBLE;
		case ExpressionType::COMPARE_DISTINCT_FROM:
			return (nstats->cluster_count == 1 && Equals::Operation(nstats->min_values[0], nstats->max_values[0]) &&
			        Equals::Operation(nstats->min_values[0], constant))
			           ? FilterPropagateResult::FILTER_ALWAYS_FALSE
					   : FilterPropagateResult::NO_PRUNING_POSSIBLE;
		case ExpressionType::COMPARE_GREATERTHANOREQUALTO:
		case ExpressionType::COMPARE_GREATERTHAN:
			return (nstats->cluster_count > 0 &&
			        GreaterThanEquals::Operation(nstats->max_values[nstats->cluster_count - 1], constant))
			           ? FilterPropagateResult::NO_PRUNING_POSSIBLE
					   : FilterPropagateResult::FILTER_ALWAYS_FALSE;
		case ExpressionType::COMPARE_LESSTHANOREQUALTO:
		case ExpressionType::COMPARE_LESSTHAN:
			return (nstats->cluster_count > 0 && LessThanEquals::Operation(nstats->min_values[0], constant))
			           ? FilterPropagateResult::NO_PRUNING_POSSIBLE
					   : FilterPropagateResult::FILTER_ALWAYS_FALSE;
			return FilterPropagateResult::NO_PRUNING_POSSIBLE;
		default:
			throw InternalException("Expression type in zonemap check not implemented");
		}
	}
	inline static FilterPropagateResult QueryRange_implementation(AdditionalStats<T> *stats, const T &start,
	                                                              const T &end) {
		ClusterAdditionalStats<T, N> *nstats = (ClusterAdditionalStats<T, N> *)stats;
		if (nstats->cluster_count == 0)
			return FilterPropagateResult::NO_PRUNING_POSSIBLE;
		int idx = FindLastIndexBeforePoint(nstats->min_values, nstats->cluster_count, start);
		if (idx == -1) {
			if (GreaterThan::Operation(nstats->min_values[0], end)) {
				return FilterPropagateResult::FILTER_ALWAYS_FALSE;
			} else {
				return FilterPropagateResult::NO_PRUNING_POSSIBLE;
			}
		} else if (GreaterThanEquals::Operation(nstats->max_values[idx], start) ||
		           (idx < nstats->cluster_count - 1 && LessThanEquals::Operation(nstats->min_values[idx + 1], end))) {
			return FilterPropagateResult::NO_PRUNING_POSSIBLE;
		} else {
			return FilterPropagateResult::FILTER_ALWAYS_FALSE;
		}
	}
	inline static size_t Size_implementation(AdditionalStats<T> *stats) {
		ClusterAdditionalStats<T, N> *nstats = (ClusterAdditionalStats<T, N> *)stats;
		return sizeof(*nstats);
	}
	inline static void Serialise_implementation(AdditionalStats<T> *stats, Serializer &serializer) {
	}
	inline static void Deserialise_implementation(AdditionalStats<T> *stats, Deserializer &deserializer) {
	}
};

struct data_array {
public:
	duckdb::data_t data[CLUSTER_MAX_STRING_MINMAX_SIZE];
	idx_t size;
};

template <unsigned int N>
class ClusterAdditionalStats<std::string, N> : public AdditionalStats<std::string> {
private:
	unsigned int cluster_count = 0;
	data_array min_values[N];
	data_array max_values[N];
	struct StringComparisonResult {
		int order;
		bool exact;
	};
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

	static StringComparisonResult StringValueComparison(const std::string &value, const data_array &bound) {
		idx_t bound_prefix_size = MinValue(bound.size, (idx_t)CLUSTER_MAX_STRING_MINMAX_SIZE);
		idx_t compare_size = MinValue(MinValue(value.size(), bound_prefix_size), (idx_t)CLUSTER_MAX_STRING_MINMAX_SIZE);
		int comparison = compare_size == 0 ? 0 : memcmp(value.data(), bound.data, compare_size);
		if (comparison != 0) {
			return StringComparisonResult {comparison < 0 ? -1 : 1, true};
		}
		if (bound.size <= CLUSTER_MAX_STRING_MINMAX_SIZE) {
			if (value.size() < bound.size) {
				return StringComparisonResult {-1, true};
			}
			return StringComparisonResult {value.size() > bound.size ? 1 : 0, true};
		}
		return StringComparisonResult {value.size() <= CLUSTER_MAX_STRING_MINMAX_SIZE ? -1 : 0,
		                               value.size() <= CLUSTER_MAX_STRING_MINMAX_SIZE};
	}

	static void ConstructValue(const_data_ptr_t data, idx_t size, data_array &target) {
		idx_t value_size = size > CLUSTER_MAX_STRING_MINMAX_SIZE ? CLUSTER_MAX_STRING_MINMAX_SIZE : size;
		memcpy(target.data, data, value_size);
		for (idx_t i = value_size; i < CLUSTER_MAX_STRING_MINMAX_SIZE; i++) {
			target.data[i] = '\0';
		}
		target.size = size;
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
		ClusterAdditionalStats<std::string, N> *nstats = (ClusterAdditionalStats<std::string, N> *)stats;
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

		for (int i = 0; i < std::min((unsigned long)gaps.size(), (unsigned long)(N - 1)); i++) {
			if (gaps[i].first == 0)
				break;
			idxs.push_back(gaps[i].second);
		}

		// save min/max ranges defined by those gaps in nstats
		std::sort(idxs.begin(), idxs.end());
		data_array start;
		data_array next;
		ConstructValue(const_data_ptr_cast(data[0].data()), data[0].size(), start);
		for (int idx : idxs) {
			nstats->min_values[nstats->cluster_count] = start;
			ConstructValue(const_data_ptr_cast(data[idx].data()), data[idx].size(), next);
			nstats->max_values[nstats->cluster_count] = next;
			ConstructValue(const_data_ptr_cast(data[idx + 1].data()), data[idx + 1].size(), start);
			nstats->cluster_count++;
		}

		nstats->min_values[nstats->cluster_count] = start;
		ConstructValue(const_data_ptr_cast(data.back().data()), data.back().size(), next);
		nstats->max_values[nstats->cluster_count] = next;
		nstats->cluster_count++;
	}

	inline static FilterPropagateResult Query_inner(const data_array &min_value, const data_array &max_value,
	                                                ExpressionType &comparison_type, const std::string &constant) {
		auto min_comp = StringValueComparison(constant, min_value);
		auto max_comp = StringValueComparison(constant, max_value);
		switch (comparison_type) {
		case ExpressionType::COMPARE_EQUAL:
		case ExpressionType::COMPARE_NOT_DISTINCT_FROM:
			if (min_comp.order >= 0 && max_comp.order <= 0) {
				return FilterPropagateResult::NO_PRUNING_POSSIBLE;
			} else {
				return FilterPropagateResult::FILTER_ALWAYS_FALSE;
			}
		case ExpressionType::COMPARE_NOTEQUAL:
		case ExpressionType::COMPARE_DISTINCT_FROM:
			if (min_comp.exact && max_comp.exact && min_comp.order == 0 && max_comp.order == 0) {
				return FilterPropagateResult::FILTER_ALWAYS_FALSE;
			}
			return FilterPropagateResult::NO_PRUNING_POSSIBLE;
		case ExpressionType::COMPARE_GREATERTHANOREQUALTO:
		case ExpressionType::COMPARE_GREATERTHAN:
			return max_comp.order > 0 ? FilterPropagateResult::FILTER_ALWAYS_FALSE
			                          : FilterPropagateResult::NO_PRUNING_POSSIBLE;
		case ExpressionType::COMPARE_LESSTHAN:
		case ExpressionType::COMPARE_LESSTHANOREQUALTO:
			return min_comp.order < 0 ? FilterPropagateResult::FILTER_ALWAYS_FALSE
			                          : FilterPropagateResult::NO_PRUNING_POSSIBLE;
		default:
			throw InternalException("Expression type not implemented for string statistics zone map");
		}
	}
	inline static FilterPropagateResult Query_implementation(AdditionalStats<std::string> *stats,
	                                                         ExpressionType &comparison_type,
	                                                         const std::string &constant) {
		ClusterAdditionalStats<std::string, N> *nstats = (ClusterAdditionalStats<std::string, N> *)stats;
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
		for (int i = 0; i < nstats->cluster_count; i++) {
			FilterPropagateResult result =
			    Query_inner(nstats->min_values[i], nstats->max_values[i], comparison_type, constant);
			if (result == FilterPropagateResult::FILTER_ALWAYS_TRUE) {
				return FilterPropagateResult::NO_PRUNING_POSSIBLE;
			} else if (result == FilterPropagateResult::NO_PRUNING_POSSIBLE)
				return FilterPropagateResult::NO_PRUNING_POSSIBLE;
		}
		return FilterPropagateResult::FILTER_ALWAYS_FALSE;
	}

	inline static FilterPropagateResult QueryRange_implementation(AdditionalStats<std::string> *stats,
	                                                              const std::string &start, const std::string &end) {
		ClusterAdditionalStats<std::string, N> *nstats = (ClusterAdditionalStats<std::string, N> *)stats;
		if (nstats->cluster_count == 0)
			return FilterPropagateResult::NO_PRUNING_POSSIBLE;
		// TODO: implement
		return FilterPropagateResult::NO_PRUNING_POSSIBLE;
	}

	inline static size_t Size_implementation(AdditionalStats<std::string> *stats) {
		ClusterAdditionalStats<std::string, N> *nstats = (ClusterAdditionalStats<std::string, N> *)stats;
		return sizeof(*nstats);
	}
	inline static void Serialise_implementation(AdditionalStats<std::string> *stats, Serializer &serializer) {
	}
	inline static void Deserialise_implementation(AdditionalStats<std::string> *stats, Deserializer &deserializer) {
	}
};

} // namespace duckdb