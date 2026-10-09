//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/storage/statistics/additional/dictionary_additional_stats.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include <functional>
#include <unordered_set>
#include "duckdb/storage/statistics/additional/additional_stats.hpp"
#include "duckdb/common/enums/filter_propagate_result.hpp"
#include "duckdb/common/operator/comparison_operators.hpp"
#include "duckdb/storage/statistics/base_statistics.hpp"
#include "duckdb/common/enums/expression_type.hpp"

#include "duckdb/common/serializer/serializer.hpp"
#include "duckdb/common/serializer/deserializer.hpp"

namespace duckdb {

template <class T, unsigned int N>
class DictionaryAdditionalStats : public AdditionalStats<T> {
private:
	bool valid = true;
	std::unordered_set<T> dictionary;

public:
	static inline const char *GetStaticName() {
		return "dictionary";
	}
	inline DictionaryAdditionalStats(std::vector<T> &data) {
		Initialise(data, this);
	}
	inline static void Initialise(std::vector<T> &data, AdditionalStats<T> *stats) {
		DictionaryAdditionalStats<T, N> *nstats = (DictionaryAdditionalStats<T, N> *)stats;
		for (T item : data) {
			nstats->dictionary.insert(item);
			if (nstats->dictionary.size() > N)
				break;
		}
		if (nstats->dictionary.size() > N) {
			nstats->valid = false;
			nstats->dictionary.clear();
			nstats->dictionary.rehash(1);
// printf("DISCARDED DICTIONARY\n");
#ifndef DEBUG
			fprintf(stderr, "%lx,%lu,%lu,DISCARDED_DICTIONARY,\"{\"\"stats\"\":\"\"%p\"\"}\"\n", Util::session_id,
			        Util::command_count, Util::GetTime(), stats);
#endif
		}
	}
	inline static FilterPropagateResult Query(AdditionalStats<T> *stats, ExpressionType &comparison_type,
	                                          const T &constant) {
		DictionaryAdditionalStats<T, N> *nstats = (DictionaryAdditionalStats<T, N> *)stats;
		if (!nstats->valid)
			return FilterPropagateResult::NO_PRUNING_POSSIBLE;
		switch (comparison_type) {
		case ExpressionType::COMPARE_EQUAL:
		case ExpressionType::COMPARE_NOT_DISTINCT_FROM:
			for (T elem : nstats->dictionary)
				if (Equals::Operation(elem, constant))
					return FilterPropagateResult::NO_PRUNING_POSSIBLE;
			return FilterPropagateResult::FILTER_ALWAYS_FALSE;
		case ExpressionType::COMPARE_NOTEQUAL:
		case ExpressionType::COMPARE_DISTINCT_FROM:
			for (T elem : nstats->dictionary)
				if (NotEquals::Operation(elem, constant))
					return FilterPropagateResult::NO_PRUNING_POSSIBLE;
			return FilterPropagateResult::FILTER_ALWAYS_FALSE;
		case ExpressionType::COMPARE_GREATERTHANOREQUALTO:
		case ExpressionType::COMPARE_GREATERTHAN:
			for (T elem : nstats->dictionary)
				if (GreaterThanEquals::Operation(elem, constant))
					return FilterPropagateResult::NO_PRUNING_POSSIBLE;
			return FilterPropagateResult::FILTER_ALWAYS_FALSE;
		case ExpressionType::COMPARE_LESSTHAN:
		case ExpressionType::COMPARE_LESSTHANOREQUALTO:
			for (T elem : nstats->dictionary)
				if (LessThanEquals::Operation(elem, constant))
					return FilterPropagateResult::NO_PRUNING_POSSIBLE;
			return FilterPropagateResult::FILTER_ALWAYS_FALSE;
		default:
			return FilterPropagateResult::NO_PRUNING_POSSIBLE;
		}
	}
	inline static FilterPropagateResult QueryRange(AdditionalStats<T> *stats, const T &start, const T &end) {
		DictionaryAdditionalStats<T, N> *nstats = (DictionaryAdditionalStats<T, N> *)stats;
		if (!nstats->valid)
			return FilterPropagateResult::NO_PRUNING_POSSIBLE;
		for (T elem : nstats->dictionary)
			if (GreaterThanEquals::Operation(elem, start) && LessThanEquals::Operation(elem, end))
				return FilterPropagateResult::NO_PRUNING_POSSIBLE;
		return FilterPropagateResult::FILTER_ALWAYS_FALSE;
	}
	inline static size_t Size(AdditionalStats<T> *stats) {
		DictionaryAdditionalStats<T, N> *nstats = (DictionaryAdditionalStats<T, N> *)stats;
		return sizeof(*nstats) + nstats->dictionary.bucket_count() * (sizeof(void *)) +
		       nstats->dictionary.size() * sizeof(T);
	}
	inline static void Serialise(AdditionalStats<T> *stats, Serializer &serializer) {
		DictionaryAdditionalStats<T, N> *nstats = (DictionaryAdditionalStats<T, N> *)stats;
		serializer.WriteProperty(1001, "dictionary:valid", nstats->valid);
		serializer.WriteProperty(1002, "dictionary:size", nstats->dictionary.size());
		for (T item : nstats->dictionary) {
			serializer.WriteProperty(1003, "dictionary:item", item);
		}
	}
	inline static void Deserialise(AdditionalStats<T> *stats, Deserializer &deserializer) {
		DictionaryAdditionalStats<T, N> *nstats = (DictionaryAdditionalStats<T, N> *)stats;
		nstats->valid = deserializer.template ReadProperty<bool>(1001, "dictionary:valid");
		auto size = deserializer.template ReadProperty<unsigned int>(1002, "dictionary:size");
		for (int i = 0; i < size; i++) {
			auto item = deserializer.template ReadProperty<T>(1003, "dictionary:item");
			nstats->dictionary.insert(item);
		}
	}
};

} // namespace duckdb