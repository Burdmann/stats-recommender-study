//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/storage/statistics/additional/bloom_additional_stats.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include <functional>
#include "duckdb/storage/statistics/additional/additional_stats.hpp"
#include "duckdb/common/enums/filter_propagate_result.hpp"
#include "duckdb/common/operator/comparison_operators.hpp"
#include "duckdb/storage/statistics/base_statistics.hpp"
#include "duckdb/common/enums/expression_type.hpp"

#include "duckdb/common/serializer/serializer.hpp"
#include "duckdb/common/serializer/deserializer.hpp"

namespace duckdb {

// N = length of substring stored, M = size of the bitmap used to encode the ngrams
template <class T, unsigned int N, unsigned int M>
class HashedNGramAdditionalStats : public AdditionalStats<T> {

public:
	static inline const char *GetStaticName() {
		return "hashed_ngram";
	}
	inline HashedNGramAdditionalStats(std::vector<T> &data) {
		Initialise(data, this);
	}

	inline static void Initialise(std::vector<T> &data, AdditionalStats<T> *stats) {
		HashedNGramAdditionalStats<T, N, M> *nstats = (HashedNGramAdditionalStats<T, N, M> *)stats;
	}

	inline static FilterPropagateResult Query(AdditionalStats<T> *stats, ExpressionType &comparison_type,
	                                          const T &constant) {
		return FilterPropagateResult::NO_PRUNING_POSSIBLE
	}

	inline static FilterPropagateResult QueryRange(AdditionalStats<T> *stats, const T &start, const T &end) {
		return FilterPropagateResult::NO_PRUNING_POSSIBLE;
	}

	inline static size_t Size(AdditionalStats<T> *stats) {
		HashedNGramAdditionalStats<T, N, M> *nstats = (HashedNGramAdditionalStats<T, N, M> *)stats;
		return sizeof(*nstats);
	}
	inline static void Serialise(AdditionalStats<T> *stats, Serializer &serializer) {
	}
	inline static void Deserialise(AdditionalStats<T> *stats, Deserializer &deserializer) {
	}
};

template <unsigned int N, unsigned int M>
class HashedNGramAdditionalStats<std::string, N, M> : public AdditionalStats<std::string> {
private:
	uint64_t bitmap[M];

public:
	static inline const char *GetStaticName() {
		return "hashed_ngram";
	}
	inline HashedNGramAdditionalStats(std::vector<std::string> &data) {
		Initialise(data, this);
	}

	inline static void Initialise(std::vector<std::string> &data, AdditionalStats<std::string> *stats) {
		HashedNGramAdditionalStats<std::string, N, M> *nstats = (HashedNGramAdditionalStats<std::string, N, M> *)stats;
		// go through all ngrams of each item and add it to the "blom filter"
	}

	inline static FilterPropagateResult Query(AdditionalStats<std::string> *stats, ExpressionType &comparison_type,
	                                          const std::string &constant) {
		// check if each ngram of constant is present in the "bloom filter"
		return FilterPropagateResult::NO_PRUNING_POSSIBLE
	}

	inline static FilterPropagateResult QueryRange(AdditionalStats<std::string> *stats, const std::string &start,
	                                               const std::string &end) {
		return FilterPropagateResult::NO_PRUNING_POSSIBLE;
	}

	inline static size_t Size(AdditionalStats<std::string> *stats) {
		HashedNGramAdditionalStats<std::string, N, M> *nstats = (HashedNGramAdditionalStats<std::string, N, M> *)stats;
		return sizeof(*nstats);
	}
	inline static void Serialise(AdditionalStats<std::string> *stats, Serializer &serializer) {
	}
	inline static void Deserialise(AdditionalStats<std::string> *stats, Deserializer &deserializer) {
	}
};

} // namespace duckdb