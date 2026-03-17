//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/storage/statistics/additional/always_prune_additional_stats.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include <functional>
#include "duckdb/storage/statistics/additional/additional_stats.hpp"
#include "duckdb/common/enums/filter_propagate_result.hpp"

namespace duckdb {

template <class T>
class AlwaysPruneAdditionalStats : public AdditionalStats<T> {
public:
	static inline const char *GetStaticName() {
		return "always prune";
	}
	inline AlwaysPruneAdditionalStats(std::vector<T> &data) {
		this->name = GetStaticName();
		this->Initialise = &Initialise_implementation;
		this->Query = &Query_implementation;
		this->QueryRange = &QueryRange_implementation;
		this->Size = &Size_implementation;
		this->Serialise = &Serialise_implementation;
		this->Deserialise = &Deserialise_implementation;
		this->Initialise(data, this);
		this->type = ADDITIONAL_STATS_TYPE::ALWAYS_PRUNE;
	}
	inline static void Initialise_implementation(std::vector<T> &data, AdditionalStats<T> *stats) {
	}
	inline static FilterPropagateResult Query_implementation(AdditionalStats<T> *stats, ExpressionType &comparison_type,
	                                                         const T &constant) {
		return FilterPropagateResult::FILTER_ALWAYS_FALSE;
	}
	inline static FilterPropagateResult QueryRange_implementation(AdditionalStats<T> *stats, const T &start,
	                                                              const T &end) {
		return FilterPropagateResult::FILTER_ALWAYS_FALSE;
	}
	inline static size_t Size_implementation(AdditionalStats<T> *stats) {
		AlwaysPruneAdditionalStats<T> *nstats = (AlwaysPruneAdditionalStats<T> *)stats;
		return sizeof(*nstats);
	}
	inline static void Serialise_implementation(AdditionalStats<T> *stats, Serializer &serializer) {
	}
	inline static void Deserialise_implementation(AdditionalStats<T> *stats, Deserializer &deserializer) {
	}
};

} // namespace duckdb