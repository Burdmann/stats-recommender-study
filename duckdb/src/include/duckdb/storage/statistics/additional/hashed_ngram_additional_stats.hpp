//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/storage/statistics/additional/hashed_ngram_additional_stats.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include <algorithm>
#include <string>
#include <vector>

#include "duckdb/storage/statistics/additional/additional_stats.hpp"
#include "duckdb/storage/statistics/additional/bloom_additional_stats.hpp"
#include "duckdb/common/enums/filter_propagate_result.hpp"
#include "duckdb/common/enums/expression_type.hpp"
#include "duckdb/common/serializer/serializer.hpp"
#include "duckdb/common/serializer/deserializer.hpp"

namespace duckdb {

// N is the byte length of each substring; M is the number of 64-bit words in the encoded statistic.
template <unsigned int N, unsigned int M, unsigned int K>
class HashedNGramAdditionalStats<std::string, N, M, K> : public AdditionalStats<std::string> {
	static_assert(N > 0, "N-gram length must be greater than zero");
	static_assert(M > 0, "The statistic must contain at least one word");

private:
	uint64_t words[M];

	static inline bool MayContainNGram(uint64_t *words, const std::string &ngram) {
		return BloomUtil<1, M, K>::QueryUtil(ngram, words);
	}

	template <class CALLBACK>
	static inline void ForEachNGram(const std::string &value, CALLBACK callback) {
		if (value.size() < N) {
			std::string padded = value;
			padded.resize(N, '\0');
			callback(padded);
			return;
		}
		for (size_t offset = 0; offset <= value.size() - N; offset++) {
			callback(value.substr(offset, N));
		}
	}

	// Returns false only if some n-gram of the literal is definitely absent. Literals shorter than N give no
	// information (no padding here: the literal is a substring of a value, not a whole value).
	static inline bool MayContainSubstring(uint64_t *words, const std::string &literal) {
		if (literal.size() < N) {
			return true;
		}
		for (size_t offset = 0; offset <= literal.size() - N; offset++) {
			if (!MayContainNGram(words, literal.substr(offset, N))) {
				return false;
			}
		}
		return true;
	}

public:
	// For prefix(), suffix() and contains(): the constant is a plain literal without wildcards.
	inline static FilterPropagateResult QueryContains(AdditionalStats<std::string> *stats,
	                                                  const std::string &literal) {
		auto nstats = (HashedNGramAdditionalStats<std::string, N, M, K> *)stats;
		if (!MayContainSubstring(nstats->words, literal)) {
			return FilterPropagateResult::FILTER_ALWAYS_FALSE;
		}
		return FilterPropagateResult::NO_PRUNING_POSSIBLE;
	}

	inline static FilterPropagateResult QueryLike(AdditionalStats<std::string> *stats, const std::string &pattern) {
		auto nstats = (HashedNGramAdditionalStats<std::string, N, M, K> *)stats;
		for (size_t run_start = 0; run_start < pattern.size();) {
			while (run_start < pattern.size() && (pattern[run_start] == '%' || pattern[run_start] == '_')) {
				run_start++;
			}
			size_t run_end = run_start;
			while (run_end < pattern.size() && pattern[run_end] != '%' && pattern[run_end] != '_') {
				run_end++;
			}
			if (!MayContainSubstring(nstats->words, pattern.substr(run_start, run_end - run_start))) {
				return FilterPropagateResult::FILTER_ALWAYS_FALSE;
			}
			run_start = run_end;
		}
		return FilterPropagateResult::NO_PRUNING_POSSIBLE;
	}

	static inline const char *GetStaticName() {
		return "hashed_ngram";
	}

	inline HashedNGramAdditionalStats(std::vector<std::string> &data) {
		Initialise(data, this);
	}

	inline static void Initialise(std::vector<std::string> &data, AdditionalStats<std::string> *stats) {
		auto nstats = (HashedNGramAdditionalStats<std::string, N, M, K> *)stats;
		std::fill(nstats->words, nstats->words + M, 0);
		for (const auto &value : data) {
			ForEachNGram(value, [&](const std::string &ngram) { BloomUtil<1, M, K>::Insert(ngram, nstats->words); });
		}
	}

	inline static FilterPropagateResult Query(AdditionalStats<std::string> *stats, ExpressionType &comparison_type,
	                                          const std::string &constant) {
		switch (comparison_type) {
		case ExpressionType::COMPARE_EQUAL:
		case ExpressionType::COMPARE_NOT_DISTINCT_FROM: {
			auto nstats = (HashedNGramAdditionalStats<std::string, N, M, K> *)stats;
			bool may_contain = true;
			bool has_ngrams = false;
			ForEachNGram(constant, [&](const std::string &ngram) {
				has_ngrams = true;
				if (may_contain && !MayContainNGram(nstats->words, ngram)) {
					may_contain = false;
				}
			});
			if (has_ngrams && !may_contain) {
				return FilterPropagateResult::FILTER_ALWAYS_FALSE;
			}
			return FilterPropagateResult::NO_PRUNING_POSSIBLE;
		}
		default:
			return FilterPropagateResult::NO_PRUNING_POSSIBLE;
		}
	}

	inline static FilterPropagateResult QueryRange(AdditionalStats<std::string> *stats, const std::string &start,
	                                               const std::string &end) {
		return FilterPropagateResult::NO_PRUNING_POSSIBLE;
	}

	inline static size_t Size(AdditionalStats<std::string> *stats) {
		auto nstats = (HashedNGramAdditionalStats<std::string, N, M, K> *)stats;
		return sizeof(*nstats);
	}

	inline static void Serialise(AdditionalStats<std::string> *stats, Serializer &serializer) {
	}

	inline static void Deserialise(AdditionalStats<std::string> *stats, Deserializer &deserializer) {
	}
};

} // namespace duckdb
