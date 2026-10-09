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
template <class T, unsigned int N, unsigned int M, unsigned int K>
class BloomAdditionalStats;
template <class T>
class AlwaysPruneAdditionalStats;
template <class T, unsigned int N>
class DictionaryAdditionalStats;

template <class T>
class AdditionalStats {
public:
	static inline const char *GetStaticName() {
		return "error";
	}
	STATISTIC_TYPE type;
};

} // namespace duckdb