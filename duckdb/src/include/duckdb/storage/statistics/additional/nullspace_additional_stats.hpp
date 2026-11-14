//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/storage/statistics/additional/nullspace_additional_stats.hpp
//
// Hash-based partition summary for point queries that comes close to the space lower bound of
// approximate membership (n * log2(1/eps) bits). Keys are hashed into buckets; per bucket and layer it
// stores a nonzero vector v with a(x) . v = 0 over GF(2) or GF(3) for every key x of the bucket, and a
// query q is pruned if a(q) . v != 0 in some layer. A non-member passes a GF(2) layer with probability
// 1/2 and a GF(3) layer with probability 1/3, at 1 resp. log2(3) bits per key. Below one bit per key it
// stores the empty cells of a hashed bitmap instead. Uses the same space as BloomAdditionalStats.
// See pruning_codec.hpp.
//===----------------------------------------------------------------------===//

#pragma once

#include <functional>
#include "duckdb/storage/statistics/additional/additional_stats.hpp"
#include "duckdb/storage/statistics/additional/bloom_additional_stats.hpp"
#include "duckdb/storage/statistics/additional/occupancy_additional_stats.hpp"
#include "duckdb/storage/statistics/additional/pruning_codec.hpp"
#include "duckdb/common/enums/filter_propagate_result.hpp"
#include "duckdb/common/enums/expression_type.hpp"
#include "duckdb/common/serializer/serializer.hpp"
#include "duckdb/common/serializer/deserializer.hpp"

namespace duckdb {

template <class T>
class NullspaceAdditionalStats : public AdditionalStats<T> {
private:
	uint64_t words[PRUNING_CODEC_WORDS];

	static inline uint64_t Hash(const T &value) {
		return pruning_codec::Mix64((uint64_t)BloomUtil::hash(value) ^ 0x2545F4914F6CDD1DULL);
	}

public:
	static inline const char *GetStaticName() {
		return "nullspace";
	}
	inline NullspaceAdditionalStats(std::vector<T> &data) {
		this->name = GetStaticName();
		this->Initialise = &Initialise_implementation;
		this->Query = &Query_implementation;
		this->QueryRange = &QueryRange_implementation;
		this->Size = &Size_implementation;
		this->Serialise = &Serialise_implementation;
		this->Deserialise = &Deserialise_implementation;
		this->Initialise(data, this);
		this->type = ADDITIONAL_STATS_TYPE::NULLSPACE;
	}

	inline static void Initialise_implementation(std::vector<T> &data, AdditionalStats<T> *stats) {
		using namespace pruning_codec;
		NullspaceAdditionalStats<T> *nstats = (NullspaceAdditionalStats<T> *)stats;
		std::fill(nstats->words, nstats->words + PRUNING_CODEC_WORDS, 0);
		if (data.empty()) {
			return; // NO_INFORMATION
		}
		std::vector<uint64_t> hashes;
		hashes.reserve(data.size());
		for (T element : data) {
			hashes.push_back(Hash(element));
		}
		std::sort(hashes.begin(), hashes.end());
		hashes.erase(std::unique(hashes.begin(), hashes.end()), hashes.end());

		// candidate 1: empty cells of a hashed bitmap (good below one bit per key)
		auto occupancy = OccupancyCodec::BestPlan(hashes, PRUNING_CODEC_BUDGET_BITS);
		long double occupied_cells = (long double)(occupancy.span_cells - occupancy.empty_cells);
		long double occupancy_eps =
		    std::min<long double>(1.0L, occupied_cells * OccupancyCodec::Width(occupancy.code) / 18446744073709551616.0L);
		// candidate 2: GF(2)/GF(3) null-space vectors (near the bound from one bit per key)
		auto nullspace = NullspaceCodec::MakePlan(hashes, PRUNING_CODEC_BUDGET_BITS);

		pruning_codec::BitWriter out;
		if (nullspace.bits <= PRUNING_CODEC_BUDGET_BITS && nullspace.expected_eps < occupancy_eps) {
			out.Write(NULLSPACE, PRUNING_CODEC_KIND_BITS);
			NullspaceCodec::Write(out, nullspace);
		} else {
			out.Write(OCCUPANCY, PRUNING_CODEC_KIND_BITS);
			OccupancyCodec::Write(out, occupancy);
		}
		StoreStream(out, nstats->words);
	}

	inline static FilterPropagateResult Query_implementation(AdditionalStats<T> *stats, ExpressionType &comparison_type,
	                                                         const T &constant) {
		using namespace pruning_codec;
		switch (comparison_type) {
		case ExpressionType::COMPARE_EQUAL:
		case ExpressionType::COMPARE_NOT_DISTINCT_FROM: {
			NullspaceAdditionalStats<T> *nstats = (NullspaceAdditionalStats<T> *)stats;
			pruning_codec::BitReader in(nstats->words);
			uint64_t kind = in.Read(PRUNING_CODEC_KIND_BITS);
			uint64_t hash = Hash(constant);
			bool may_contain = kind == NULLSPACE   ? NullspaceCodec::MayContain(in, hash)
			                   : kind == OCCUPANCY ? OccupancyCodec::MayContain(in, hash, hash)
			                                       : true;
			return may_contain ? FilterPropagateResult::NO_PRUNING_POSSIBLE : FilterPropagateResult::FILTER_ALWAYS_FALSE;
		}
		default:
			return FilterPropagateResult::NO_PRUNING_POSSIBLE;
		}
	}

	inline static FilterPropagateResult QueryRange_implementation(AdditionalStats<T> *stats, const T &start,
	                                                              const T &end) {
		return FilterPropagateResult::NO_PRUNING_POSSIBLE;
	}

	inline static size_t Size_implementation(AdditionalStats<T> *stats) {
		NullspaceAdditionalStats<T> *nstats = (NullspaceAdditionalStats<T> *)stats;
		return sizeof(*nstats);
	}
	inline static void Serialise_implementation(AdditionalStats<T> *stats, Serializer &serializer) {
	}
	inline static void Deserialise_implementation(AdditionalStats<T> *stats, Deserializer &deserializer) {
	}
};

} // namespace duckdb
