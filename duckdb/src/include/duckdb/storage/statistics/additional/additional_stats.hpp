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
enum class STATISTIC_TYPE : int8_t;

template <class T>
class EmptyAdditionalStats;
template <class T, unsigned int N>
class ClusterAdditionalStats;
template <class T, unsigned int N, unsigned int M>
class BloomAdditionalStats;
template <class T>
class AlwaysPruneAdditionalStats;
template <class T, unsigned int N>
class DictionaryAdditionalStats;

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
	STATISTIC_TYPE type;

	AdditionalStats() {
		this->name = GetStaticName();
	}
};

} // namespace duckdb