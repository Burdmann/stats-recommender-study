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

namespace duckdb {
class AdditionalStatsUtil {
public:
	template <class T>
	static inline void SerialiseStats(Serializer &serializer, AdditionalStats<T> *astats) {
		serializer.WriteProperty(1000, "additional_stats_type", (uint8_t)((AdditionalStats<uint32_t> *)astats)->type);
		astats->Serialise(astats, serializer);
	}
	template <class T>
	static inline AdditionalStats<T> *DeserialiseStats(Deserializer &deserializer) {
		auto additional_stats_type =
		    (ADDITIONAL_STATS_TYPE)deserializer.ReadProperty<uint8_t>(1000, "additional_stats_type");
		std::vector<T> empty;
		AdditionalStats<T> *astats;
		switch (additional_stats_type) {
		case ADDITIONAL_STATS_TYPE::ERROR:
			// throw exception
			break;
		case ADDITIONAL_STATS_TYPE::EMPTY:
			astats = new EmptyAdditionalStats<T>(empty);
			astats->Deserialise(astats, deserializer);
			break;
		case ADDITIONAL_STATS_TYPE::CLUSTER:
			astats = new ClusterAdditionalStats<T>(empty);
			astats->Deserialise(astats, deserializer);
			break;
		case ADDITIONAL_STATS_TYPE::BLOOM:
			astats = new BloomAdditionalStats<T>(empty);
			astats->Deserialise(astats, deserializer);
			break;
		case ADDITIONAL_STATS_TYPE::DICTIONARY:
			astats = new DictionaryAdditionalStats<T>(empty);
			astats->Deserialise(astats, deserializer);
			break;
		case ADDITIONAL_STATS_TYPE::ALWAYS_PRUNE:
			astats = new AlwaysPruneAdditionalStats<T>(empty);
			astats->Deserialise(astats, deserializer);
			break;
		}
		return astats;
	}
};

} // namespace duckdb
