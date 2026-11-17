//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/storage/statistics/additional/occupancy_additional_stats.hpp
//
// Order-preserving partition summary for range (and point) queries: the key domain is split into
// cells of equal width, and the occupied cells are stored as a Golomb-coded gap list. The cell width is
// the finest one whose encoding fits in the same space as BloomAdditionalStats. A range is pruned if
// every cell it touches is empty. See pruning_codec.hpp.
//===----------------------------------------------------------------------===//

#pragma once

#include <functional>
#include "duckdb/storage/statistics/additional/additional_stats.hpp"
#include "duckdb/storage/statistics/additional/pruning_codec.hpp"
#include "duckdb/common/enums/filter_propagate_result.hpp"
#include "duckdb/common/enums/expression_type.hpp"
#include "duckdb/common/serializer/serializer.hpp"
#include "duckdb/common/serializer/deserializer.hpp"

namespace duckdb {

namespace pruning_codec {

// Order-preserving map to uint64_t; false for types without one (no pruning for those).
template <class T>
inline bool OrderedKey(const T &, uint64_t &) {
	return false;
}
inline bool OrderedKey(const bool &value, uint64_t &key) {
	key = value ? 1 : 0;
	return true;
}
#define PRUNING_CODEC_SIGNED_KEY(TYPE)                                                                                 \
	inline bool OrderedKey(const TYPE &value, uint64_t &key) {                                                         \
		key = (uint64_t)(int64_t)value ^ (1ULL << 63);                                                                 \
		return true;                                                                                                   \
	}
#define PRUNING_CODEC_UNSIGNED_KEY(TYPE)                                                                               \
	inline bool OrderedKey(const TYPE &value, uint64_t &key) {                                                         \
		key = (uint64_t)value;                                                                                         \
		return true;                                                                                                   \
	}
PRUNING_CODEC_SIGNED_KEY(int8_t)
PRUNING_CODEC_SIGNED_KEY(int16_t)
PRUNING_CODEC_SIGNED_KEY(int32_t)
PRUNING_CODEC_SIGNED_KEY(int64_t)
PRUNING_CODEC_UNSIGNED_KEY(uint8_t)
PRUNING_CODEC_UNSIGNED_KEY(uint16_t)
PRUNING_CODEC_UNSIGNED_KEY(uint32_t)
PRUNING_CODEC_UNSIGNED_KEY(uint64_t)
#undef PRUNING_CODEC_SIGNED_KEY
#undef PRUNING_CODEC_UNSIGNED_KEY
inline bool OrderedKey(const double &value, uint64_t &key) {
	if (value != value) {
		key = ~0ULL; // NaN sorts last
		return true;
	}
	double normalised = value == 0 ? 0.0 : value; // -0.0 == 0.0
	uint64_t bits;
	memcpy(&bits, &normalised, sizeof(bits));
	key = (bits >> 63) ? ~bits : bits | (1ULL << 63);
	return true;
}
inline bool OrderedKey(const float &value, uint64_t &key) {
	return OrderedKey((double)value, key);
}

// Stats layout shared by both statistics: 2 bits of kind, then the codec stream.
enum PruningCodecKind : uint64_t { NO_INFORMATION = 0, OCCUPANCY = 1, NULLSPACE = 2 };

inline void StoreStream(const BitWriter &out, uint64_t *words, uint PRUNING_CODEC_WORDS) {
	std::fill(words, words + PRUNING_CODEC_WORDS, 0);
	if (out.bits > PRUNING_CODEC_WORDS * 64) {
		return; // does not fit (cannot happen): NO_INFORMATION
	}
	std::copy(out.words.begin(), out.words.end(), words);
}

} // namespace pruning_codec

template <class T, uint PRUNING_CODEC_WORDS>
class OccupancyAdditionalStats : public AdditionalStats<T> {
private:
	uint64_t words[PRUNING_CODEC_WORDS];
	constexpr static uint64_t PRUNING_CODEC_KIND_BITS = 2;
	constexpr static uint64_t PRUNING_CODEC_BUDGET_BITS = PRUNING_CODEC_WORDS * 64 - PRUNING_CODEC_KIND_BITS;

public:
	static inline const char *GetStaticName() {
		return "occupancy";
	}
	inline OccupancyAdditionalStats(std::vector<T> &data) {
		this->Initialise(data, this);
	}

	inline static void Initialise(std::vector<T> &data, AdditionalStats<T> *stats) {
		using namespace pruning_codec;
		OccupancyAdditionalStats<T, PRUNING_CODEC_WORDS> *nstats =
		    (OccupancyAdditionalStats<T, PRUNING_CODEC_WORDS> *)stats;
		std::fill(nstats->words, nstats->words + PRUNING_CODEC_WORDS, 0);
		std::vector<uint64_t> keys;
		keys.reserve(data.size());
		for (T element : data) {
			uint64_t key;
			if (!OrderedKey(element, key)) {
				return; // NO_INFORMATION
			}
			keys.push_back(key);
		}
		if (keys.empty()) {
			return;
		}
		std::sort(keys.begin(), keys.end());
		keys.erase(std::unique(keys.begin(), keys.end()), keys.end());
		auto plan = OccupancyCodec::BestPlan(keys, PRUNING_CODEC_BUDGET_BITS);
		pruning_codec::BitWriter out;
		out.Write(OCCUPANCY, PRUNING_CODEC_KIND_BITS);
		OccupancyCodec::Write(out, plan);
		StoreStream(out, nstats->words, PRUNING_CODEC_WORDS);
	}

	inline static bool MayContain(AdditionalStats<T> *stats, const T &start, const T &end) {
		using namespace pruning_codec;
		OccupancyAdditionalStats<T, PRUNING_CODEC_WORDS> *nstats =
		    (OccupancyAdditionalStats<T, PRUNING_CODEC_WORDS> *)stats;
		pruning_codec::BitReader in(nstats->words);
		if (in.Read(PRUNING_CODEC_KIND_BITS) != OCCUPANCY) {
			return true;
		}
		uint64_t lo, hi;
		if (!OrderedKey(start, lo) || !OrderedKey(end, hi) || lo > hi) {
			return true;
		}
		return OccupancyCodec::MayContain(in, lo, hi);
	}

	inline static FilterPropagateResult Query(AdditionalStats<T> *stats, ExpressionType &comparison_type,
	                                          const T &constant) {
		switch (comparison_type) {
		case ExpressionType::COMPARE_EQUAL:
		case ExpressionType::COMPARE_NOT_DISTINCT_FROM:
			return MayContain(stats, constant, constant) ? FilterPropagateResult::NO_PRUNING_POSSIBLE
			                                             : FilterPropagateResult::FILTER_ALWAYS_FALSE;
		default:
			// one-sided comparisons are answered exactly by min/max
			return FilterPropagateResult::NO_PRUNING_POSSIBLE;
		}
	}

	inline static FilterPropagateResult QueryRange(AdditionalStats<T> *stats, const T &start, const T &end) {
		return MayContain(stats, start, end) ? FilterPropagateResult::NO_PRUNING_POSSIBLE
		                                     : FilterPropagateResult::FILTER_ALWAYS_FALSE;
	}

	inline static size_t Size(AdditionalStats<T> *stats) {
		OccupancyAdditionalStats<T, PRUNING_CODEC_WORDS> *nstats =
		    (OccupancyAdditionalStats<T, PRUNING_CODEC_WORDS> *)stats;
		return sizeof(*nstats);
	}
	inline static void Serialise(AdditionalStats<T> *stats, Serializer &serializer) {
	}
	inline static void Deserialise(AdditionalStats<T> *stats, Deserializer &deserializer) {
	}
};

} // namespace duckdb
