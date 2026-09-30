//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/storage/statistics/additional/additional_stats_util.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once
#include "duckdb/storage/statistics/additional/empty_additional_stats.hpp"
#include "duckdb/storage/statistics/additional/cluster_additional_stats.hpp"
#include "duckdb/storage/statistics/additional/bloom_additional_stats.hpp"
#include "duckdb/storage/statistics/additional/always_prune_additional_stats.hpp"
#include "duckdb/storage/statistics/additional/dictionary_additional_stats.hpp"
#include "duckdb/common/serializer/serializer.hpp"
#include "duckdb/common/serializer/deserializer.hpp"
#include "duckdb/storage/statistics/additional/stats_set.hpp"

namespace duckdb {
class AdditionalStatsUtil {
public:
	template <class T>
	static inline void SerialiseStats(Serializer &serializer, AdditionalStats<T> *astats) {
		serializer.WriteProperty(1000, "additional_stats_type", (int8_t)((AdditionalStats<uint32_t> *)astats)->type);
		astats->Serialise(astats, serializer);
	}
	template <class T>
	static inline AdditionalStats<T> *DeserialiseStats(Deserializer &deserializer) {
		auto additional_stats_type = (STATISTIC_TYPE)deserializer.ReadProperty<int8_t>(1000, "additional_stats_type");
		std::vector<T> empty;
		AdditionalStats<T> *astats;
		astats = StatisticsSet::construct(empty, additional_stats_type);
		astats->Deserialise(astats, deserializer);
		return astats;
	}
};

} // namespace duckdb
