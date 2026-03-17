//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/storage/statistics/additional/additional_stats.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include <functional>
#include <memory>
// #include "duckdb/common/serializer/serializer.hpp"
// #include "duckdb/common/serializer/deserializer.hpp"

namespace duckdb {
template <class T>
class EmptyAdditionalStats;
template <class T>
class ClusterAdditionalStats;
template <class T>
class BloomAdditionalStats;
template <class T>
class AlwaysPruneAdditionalStats;
template <class T>
class DictionaryAdditionalStats;

enum class ADDITIONAL_STATS_TYPE : uint8_t {
	ERROR = 0,
	EMPTY = 1,
	CLUSTER = 2,
	BLOOM = 3,
	DICTIONARY = 4,
	ALWAYS_PRUNE = 5,
};

template <class T>
class AdditionalStats {
public:
	std::function<void(std::vector<T> &, AdditionalStats<T> *)> Initialise;
	std::function<FilterPropagateResult(AdditionalStats<T> *, ExpressionType &, const T &)> Query;
	std::function<FilterPropagateResult(AdditionalStats<T> *, const T &, const T &)> QueryRange;
	std::function<size_t(AdditionalStats<T> *)> Size;
	std::function<void(AdditionalStats<T> *, Serializer &)> Serialise;
	std::function<void(AdditionalStats<T> *, Deserializer &)> Deserialise;
	static inline const char *GetStaticName() {
		return "error";
	}
	const char *name;
	ADDITIONAL_STATS_TYPE type;

	AdditionalStats() {
		this->name = GetStaticName();
		type = ADDITIONAL_STATS_TYPE::ERROR;
	}
	// init
	// void Initialise(std::vector<T> &data) {

	// }
	// query
	// FilterPropagateResult Query() {

	// }
	// size
	// size_t Size() {

	// }
	// serialise
	// void Serialise() {

	// }
	// deserialise
	// void Deserialise() {

	// }
};

} // namespace duckdb