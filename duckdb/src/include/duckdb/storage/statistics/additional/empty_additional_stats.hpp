//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/storage/statistics/additional/empty_additional_stats.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include <functional>
#include "duckdb/storage/statistics/additional/additional_stats.hpp"
#include "duckdb/common/enums/filter_propagate_result.hpp"

namespace duckdb {

template <class T>
class EmptyAdditionalStats : public AdditionalStats<T> {
public:
	static inline const char *GetStaticName() {
		return "empty";
	}
	inline EmptyAdditionalStats(std::vector<T> &data) {
		Initialise(data, this);
	}
	inline static void Initialise(std::vector<T> &data, AdditionalStats<T> *stats) {
	}
	inline static FilterPropagateResult Query(AdditionalStats<T> *stats, ExpressionType &comparison_type,
	                                          const T &constant) {
		return FilterPropagateResult::NO_PRUNING_POSSIBLE;
	}
	inline static FilterPropagateResult QueryRange(AdditionalStats<T> *stats, const T &start, const T &end) {
		return FilterPropagateResult::NO_PRUNING_POSSIBLE;
	}
	inline static size_t Size(AdditionalStats<T> *stats) {
		EmptyAdditionalStats<T> *nstats = (EmptyAdditionalStats<T> *)stats;
		return sizeof(*nstats);
	}
	inline static void Serialise(AdditionalStats<T> *stats, Serializer &serializer) {
	}
	inline static void Deserialise(AdditionalStats<T> *stats, Deserializer &deserializer) {
	}
};

} // namespace duckdb