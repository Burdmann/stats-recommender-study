//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/storage/statistics/additional/pruning_codec.hpp
//
// Space-efficient partition summaries used by OccupancyAdditionalStats (range and point queries) and
// NullspaceAdditionalStats (point queries). This header has no DuckDB dependencies, so the codecs can
// be tested stand-alone.
//
// Background (point queries): a summary of n keys that lets a non-member pass with probability eps
// needs at least n * log2(1/eps) bits. A Bloom filter with k = 1 is far from that bound, especially
// at small sizes. The two codecs here get close to it:
//
//  * OccupancyCodec: split the (order-preserving) key domain into cells of width b, and store the
//    occupied cells as a Golomb-coded gap list (of the occupied cells, or of the empty cells, whichever
//    is shorter). b is the finest width whose encoding fits in the budget. A query range is pruned if
//    every cell it touches is empty. Since it preserves order, it also answers range queries.
//
//  * NullspaceCodec: keys are hashed into buckets of about 256 keys. For every bucket and layer, a
//    nonzero vector v with a(x) . v = 0 for every key x of the bucket is stored, where a(x) is a
//    pseudo-random coefficient vector over GF(2) or GF(3) with (keys + 1) cells. A query q can only be
//    in the partition if a(q) . v = 0 in every layer, so there are no false negatives, and a non-member
//    passes a GF(2) layer with probability 1/2 (1 bit per key) and a GF(3) layer with probability 1/3
//    (log2(3) bits per key). Both lie on the lower bound; mixing layers per bucket fills the budget.
//    Below one bit per key, a hashed OccupancyCodec is used instead (whichever prunes more).
//===----------------------------------------------------------------------===//

#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <vector>

namespace duckdb {
namespace pruning_codec {

static inline uint64_t Mix64(uint64_t x) {
	x ^= x >> 30;
	x *= 0xbf58476d1ce4e5b9ULL;
	x ^= x >> 27;
	x *= 0x94d049bb133111ebULL;
	x ^= x >> 31;
	return x;
}

static inline uint64_t MulHi(uint64_t a, uint64_t b) {
	return (uint64_t)(((unsigned __int128)a * b) >> 64);
}

static inline uint32_t BitLength(uint64_t x) {
	return x == 0 ? 0 : 64 - __builtin_clzll(x);
}

//===--------------------------------------------------------------------===//
// Bit streams (MSB-first)
//===--------------------------------------------------------------------===//
class BitWriter {
public:
	std::vector<uint64_t> words;
	uint64_t bits = 0;

	void Write(uint64_t value, uint32_t count) {
		// writes the lowest `count` bits of value, most significant first
		while (count > 0) {
			uint32_t offset = bits & 63;
			if (offset == 0) {
				words.push_back(0);
			}
			uint32_t room = 64 - offset;
			uint32_t take = count < room ? count : room;
			uint64_t chunk = (value >> (count - take)) & (take == 64 ? ~0ULL : ((1ULL << take) - 1));
			words.back() |= chunk << (room - take);
			bits += take;
			count -= take;
		}
	}
	void WriteOnes(uint64_t count) {
		while (count >= 64) {
			Write(~0ULL, 64);
			count -= 64;
		}
		Write((1ULL << count) - 1, (uint32_t)count);
	}
	// length-prefixed value: 7 bits of length, then the value
	void WriteVar(uint64_t value) {
		uint32_t len = BitLength(value);
		Write(len, 7);
		Write(value, len);
	}
};

class BitReader {
public:
	const uint64_t *words;
	uint64_t pos;

	BitReader(const uint64_t *words, uint64_t pos = 0) : words(words), pos(pos) {
	}

	uint64_t Read(uint32_t count) {
		uint64_t result = 0;
		while (count > 0) {
			uint32_t offset = pos & 63;
			uint32_t room = 64 - offset;
			uint32_t take = count < room ? count : room;
			uint64_t chunk = (words[pos >> 6] << offset) >> (64 - take);
			result = take == 64 ? chunk : (result << take) | chunk;
			pos += take;
			count -= take;
		}
		return result;
	}
	// number of ones before the next zero (the zero is consumed)
	uint64_t ReadUnary() {
		uint64_t count = 0;
		while (true) {
			uint32_t offset = pos & 63;
			uint64_t rest = ~(words[pos >> 6] << offset); // first zero bit becomes the first one bit
			uint32_t room = 64 - offset;
			uint32_t ones = rest == 0 ? 64 : __builtin_clzll(rest);
			if (ones < room) {
				pos += ones + 1;
				return count + ones;
			}
			count += room;
			pos += room;
		}
	}
	uint64_t ReadVar() {
		return Read((uint32_t)Read(7));
	}
};

//===--------------------------------------------------------------------===//
// Golomb codes with truncated binary remainders
//===--------------------------------------------------------------------===//
struct Golomb {
	uint64_t m;
	uint32_t b;   // ceil(log2(m))
	uint64_t cut; // 2^b - m

	explicit Golomb(uint64_t m) : m(m) {
		b = m <= 1 ? 0 : BitLength(m - 1);
		cut = b == 0 ? 0 : (b == 64 ? (0 - m) : (1ULL << b) - m);
	}
	uint64_t Cost(uint64_t gap) const {
		uint64_t q = gap / m, r = gap % m;
		return q + 1 + (b == 0 ? 0 : (r < cut ? b - 1 : b));
	}
	void Encode(BitWriter &out, uint64_t gap) const {
		uint64_t q = gap / m, r = gap % m;
		out.WriteOnes(q);
		out.Write(0, 1);
		if (b == 0) {
			return;
		}
		if (r < cut) {
			out.Write(r, b - 1);
		} else {
			out.Write(r + cut, b);
		}
	}
	uint64_t Decode(BitReader &in) const {
		uint64_t q = in.ReadUnary();
		if (b == 0) {
			return q * m;
		}
		uint64_t x = b > 1 ? in.Read(b - 1) : 0;
		if (x >= cut) {
			x = ((x << 1) | in.Read(1)) - cut;
		}
		return q * m + x;
	}
};

// Golomb parameter with the smallest total cost for these gaps (searched around the optimum for a
// geometric distribution with the same mean).
static inline uint64_t BestGolomb(const std::vector<uint64_t> &gaps, uint64_t &best_cost) {
	best_cost = 0;
	if (gaps.empty()) {
		return 1;
	}
	long double sum = 0;
	for (auto g : gaps) {
		sum += g;
	}
	long double mean = sum / gaps.size();
	long double p = 1.0L / (mean + 1.0L);
	long double guess = p >= 1.0L ? 1.0L : std::ceil(std::log(2.0L - p) / -std::log1p(-p));
	uint64_t center = guess < 1 ? 1 : (guess > 1e18L ? (uint64_t)1e18 : (uint64_t)guess);
	uint64_t best_m = 1;
	best_cost = ~0ULL;
	for (int64_t delta = -3; delta <= 3; delta++) {
		int64_t m = (int64_t)center + delta;
		if (m < 1) {
			continue;
		}
		Golomb code((uint64_t)m);
		uint64_t cost = 0;
		for (auto g : gaps) {
			cost += code.Cost(g);
		}
		if (cost < best_cost) {
			best_cost = cost;
			best_m = (uint64_t)m;
		}
	}
	return best_m;
}

//===--------------------------------------------------------------------===//
// OccupancyCodec: Golomb-coded occupied (or empty) cells of width b
//===--------------------------------------------------------------------===//
// Layout: width code (12 bits), first cell (var), last - first (var), mode (1 bit: 0 = gaps between
// occupied cells, 1 = gaps between empty cells), Golomb parameter (var), gaps. The first and the last
// cell are always occupied. In mode 0 the gaps lead from the first cell to the last; in mode 1 a final
// gap leads to the last cell and ends the list.
class OccupancyCodec {
public:
	static constexpr uint32_t WIDTH_CODE_BITS = 12;
	static constexpr uint32_t WIDTH_CODES = 58 * 64;

	// cell widths grow by ~1.5% per code: (64 + mantissa) * 2^exponent / 64
	static inline uint64_t Width(uint32_t code) {
		uint64_t mantissa = 64 + (code & 63);
		uint32_t exponent = code >> 6;
		uint64_t width = (uint64_t)(((unsigned __int128)mantissa << exponent) >> 6);
		return width == 0 ? 1 : width;
	}

	struct Plan {
		uint32_t code = 0;
		uint64_t first = 0, last = 0;
		bool empty_mode = false;
		uint64_t golomb = 1;
		std::vector<uint64_t> gaps;
		uint64_t bits = 0;
		uint64_t span_cells = 0, empty_cells = 0;
	};

	// sorted distinct keys, at least one
	static Plan MakePlan(const std::vector<uint64_t> &keys, uint32_t code) {
		Plan plan;
		plan.code = code;
		uint64_t width = Width(code);
		plan.first = keys.front() / width;
		plan.last = keys.back() / width;
		plan.span_cells = plan.last - plan.first + 1;
		std::vector<uint64_t> occupied_gaps;
		std::vector<uint64_t> empty_gaps;
		uint64_t prev = plan.first;
		uint64_t prev_empty = plan.first; // previous empty cell, or the first cell
		uint64_t occupied = 1;
		for (auto key : keys) {
			uint64_t cell = key / width;
			if (cell == prev) {
				continue;
			}
			occupied++;
			occupied_gaps.push_back(cell - prev - 1);
			// empty cells strictly between prev and cell
			for (uint64_t e = prev + 1; e < cell; e++) {
				empty_gaps.push_back(e - prev_empty - 1);
				prev_empty = e;
				if (empty_gaps.size() > keys.size() * 4 + 64) {
					break; // too many empty cells to be the cheaper mode; cost is computed as infinite
				}
			}
			prev = cell;
		}
		plan.empty_cells = plan.span_cells - occupied;
		bool empty_complete = empty_gaps.size() == plan.empty_cells;
		if (plan.last != plan.first && empty_complete) {
			empty_gaps.push_back(plan.last - prev_empty - 1); // terminating gap to the last cell
		}
		uint64_t occupied_cost = 0, empty_cost = ~0ULL;
		uint64_t occupied_m = BestGolomb(occupied_gaps, occupied_cost);
		uint64_t empty_m = 1;
		if (empty_complete) {
			empty_m = BestGolomb(empty_gaps, empty_cost);
		}
		plan.empty_mode = empty_cost < occupied_cost;
		plan.golomb = plan.empty_mode ? empty_m : occupied_m;
		plan.gaps = plan.empty_mode ? std::move(empty_gaps) : std::move(occupied_gaps);
		uint64_t gap_bits = plan.empty_mode ? empty_cost : occupied_cost;
		plan.bits = WIDTH_CODE_BITS + 7 + BitLength(plan.first) + 7 + BitLength(plan.last - plan.first) + 1 + 7 +
		            BitLength(plan.golomb) + gap_bits;
		return plan;
	}

	// Finest cell width whose encoding fits in budget_bits (keys: sorted, distinct, non-empty).
	static Plan BestPlan(const std::vector<uint64_t> &keys, uint64_t budget_bits) {
		uint32_t lo = 0, hi = WIDTH_CODES - 1;
		while (lo < hi) {
			uint32_t mid = (lo + hi) / 2;
			if (MakePlan(keys, mid).bits <= budget_bits) {
				hi = mid;
			} else {
				lo = mid + 1;
			}
		}
		// the cost is not strictly monotone in the width; look a little finer
		Plan best = MakePlan(keys, lo);
		for (uint32_t code = lo > 8 ? lo - 8 : 0; code < lo; code++) {
			Plan plan = MakePlan(keys, code);
			if (plan.bits <= budget_bits) {
				return plan;
			}
		}
		return best;
	}

	static void Write(BitWriter &out, const Plan &plan) {
		out.Write(plan.code, WIDTH_CODE_BITS);
		out.WriteVar(plan.first);
		out.WriteVar(plan.last - plan.first);
		out.Write(plan.empty_mode ? 1 : 0, 1);
		out.WriteVar(plan.golomb);
		Golomb code(plan.golomb);
		for (auto gap : plan.gaps) {
			code.Encode(out, gap);
		}
	}

	// true if some key in [lo, hi] may exist
	static bool MayContain(BitReader in, uint64_t lo, uint64_t hi) {
		uint64_t width = Width((uint32_t)in.Read(WIDTH_CODE_BITS));
		uint64_t first = in.ReadVar();
		uint64_t last = first + in.ReadVar();
		uint64_t from = lo / width, to = hi / width;
		if (to < first || from > last) {
			return false;
		}
		from = std::max(from, first);
		to = std::min(to, last);
		if (from == first || to == last) {
			return true;
		}
		bool empty_mode = in.Read(1) != 0;
		Golomb code(in.ReadVar());
		uint64_t cell = first;
		if (!empty_mode) {
			// walk the occupied cells until reaching `from`
			while (true) {
				cell += code.Decode(in) + 1;
				if (cell >= from) {
					return cell <= to;
				}
			}
		}
		// walk the empty cells: every cell in [from, to] must be one of them
		uint64_t needed = from;
		while (true) {
			cell += code.Decode(in) + 1;
			if (cell < needed) {
				continue;
			}
			if (cell != needed) {
				return true; // `needed` is occupied (this includes reaching the last cell)
			}
			if (++needed > to) {
				return false;
			}
		}
	}
};

//===--------------------------------------------------------------------===//
// NullspaceCodec
//===--------------------------------------------------------------------===//
class NullspaceCodec {
public:
	// Bucket sizes: 64 << code keys. Elimination time grows with the square of the bucket size per key,
	// the per-bucket overhead (one cell per layer, the bucket count) shrinks with it.
	static constexpr uint32_t BUCKET_CODE_BITS = 2;
	static constexpr uint32_t MAX_GF2 = 31;    // 5 bits
	static constexpr uint32_t MAX_GF3 = 3;     // 2 bits
	static constexpr uint32_t TRIT_CHUNK = 80; // 3^80 < 2^127
	static constexpr uint32_t TRIT_CHUNK_BITS = 127;

	struct Mode {
		uint32_t gf2;
		uint32_t gf3;
	};

	// bits needed for t trits: ceil(t * log2(3)), with chunks of 80 trits in 127 bits
	static uint64_t TritBits(uint64_t trits) {
		static uint32_t tail_bits[TRIT_CHUNK];
		static bool initialised = false;
		if (!initialised) {
			for (uint32_t t = 0; t < TRIT_CHUNK; t++) {
				unsigned __int128 power = 1;
				for (uint32_t i = 0; i < t; i++) {
					power *= 3;
				}
				uint32_t bits = 0;
				while (bits < 128 && ((unsigned __int128)1 << bits) < power) {
					bits++;
				}
				tail_bits[t] = bits;
			}
			initialised = true;
		}
		return (trits / TRIT_CHUNK) * TRIT_CHUNK_BITS + tail_bits[trits % TRIT_CHUNK];
	}

	static uint32_t BucketCode(uint64_t keys, uint64_t budget_bits) {
		return budget_bits >= 6 * keys ? 0 : 2; // 64 keys when there are many layers, else 256
	}
	static uint64_t BucketCount(uint64_t keys, uint32_t code) {
		uint64_t bucket_keys = 64ULL << code;
		uint64_t buckets = (keys + bucket_keys / 2) / bucket_keys;
		return buckets == 0 ? 1 : buckets;
	}
	static uint64_t BucketOf(uint64_t hash, uint64_t buckets) {
		return MulHi(hash, buckets);
	}
	static uint64_t LayerSeed(uint64_t hash, uint32_t layer) {
		return Mix64(hash ^ (0x9E3779B97F4A7C15ULL * (layer + 1)));
	}
	static uint64_t ModeBits(const Mode &mode, uint64_t cells) {
		return cells * mode.gf2 + TritBits(cells) * mode.gf3;
	}
	static double ModeEps(const Mode &mode) {
		return std::ldexp(1.0, -(int)mode.gf2) * (mode.gf3 == 0   ? 1.0
		                                          : mode.gf3 == 1 ? 1.0 / 3
		                                          : mode.gf3 == 2 ? 1.0 / 9
		                                                          : 1.0 / 27);
	}

	// pseudo-random GF(2) row of `cells` bits
	static void Gf2Row(uint64_t seed, uint64_t cells, uint64_t *row) {
		uint64_t words = (cells + 63) / 64;
		for (uint64_t w = 0; w < words; w++) {
			seed += 0x9E3779B97F4A7C15ULL;
			row[w] = Mix64(seed);
		}
		if (cells & 63) {
			row[words - 1] &= (1ULL << (cells & 63)) - 1;
		}
	}
	// pseudo-random GF(3) row of `cells` trits (20 trits per 64-bit draw)
	static void Gf3Row(uint64_t seed, uint64_t cells, uint8_t *row) {
		uint64_t word = 0;
		for (uint64_t i = 0; i < cells; i++) {
			if (i % 20 == 0) {
				seed += 0x9E3779B97F4A7C15ULL;
				word = Mix64(seed);
			}
			unsigned __int128 product = (unsigned __int128)word * 3;
			row[i] = (uint8_t)(product >> 64);
			word = (uint64_t)product;
		}
	}

	// Nonzero v with a(x) . v = 0 over GF(2) for all keys; cells = keys + 1.
	static std::vector<uint64_t> SolveGf2(const std::vector<uint64_t> &hashes, uint32_t layer, uint64_t cells) {
		uint64_t words = (cells + 63) / 64;
		uint64_t rows = hashes.size();
		std::vector<uint64_t> matrix(rows * words);
		for (uint64_t r = 0; r < rows; r++) {
			Gf2Row(LayerSeed(hashes[r], layer), cells, &matrix[r * words]);
		}
		std::vector<int64_t> pivot_col(rows, -1);
		std::vector<bool> is_pivot(cells, false);
		uint64_t rank = 0;
		for (uint64_t col = 0; col < cells && rank < rows; col++) {
			uint64_t w = col >> 6, bit = 1ULL << (col & 63);
			uint64_t found = rows;
			for (uint64_t r = rank; r < rows; r++) {
				if (matrix[r * words + w] & bit) {
					found = r;
					break;
				}
			}
			if (found == rows) {
				continue;
			}
			if (found != rank) {
				std::swap_ranges(&matrix[found * words], &matrix[found * words] + words, &matrix[rank * words]);
			}
			const uint64_t *pivot = &matrix[rank * words];
			for (uint64_t r = 0; r < rows; r++) {
				if (r != rank && (matrix[r * words + w] & bit)) {
					uint64_t *row = &matrix[r * words];
					for (uint64_t i = w; i < words; i++) {
						row[i] ^= pivot[i];
					}
				}
			}
			pivot_col[rank] = (int64_t)col;
			is_pivot[col] = true;
			rank++;
		}
		// random free variables, not all zero
		std::vector<uint64_t> v(words, 0);
		uint64_t seed = Mix64(hashes.empty() ? 1 : hashes[0] + layer + 12345);
		bool any = false;
		int64_t first_free = -1;
		for (uint64_t col = 0; col < cells; col++) {
			if (is_pivot[col]) {
				continue;
			}
			if (first_free < 0) {
				first_free = (int64_t)col;
			}
			seed = Mix64(seed + col);
			if (seed & 1) {
				v[col >> 6] |= 1ULL << (col & 63);
				any = true;
			}
		}
		if (!any) {
			v[first_free >> 6] |= 1ULL << (first_free & 63);
		}
		for (uint64_t r = 0; r < rank; r++) {
			const uint64_t *row = &matrix[r * words];
			uint64_t parity = 0;
			for (uint64_t i = 0; i < words; i++) {
				parity ^= row[i] & v[i];
			}
			if (__builtin_popcountll(parity) & 1) {
				uint64_t col = (uint64_t)pivot_col[r];
				v[col >> 6] |= 1ULL << (col & 63);
			}
		}
		return v;
	}

	// Nonzero v with a(x) . v = 0 over GF(3) for all keys; cells = keys + 1. Rows are bit-sliced: plane 0
	// marks the cells equal to 1, plane 1 the cells equal to 2.
	static std::vector<uint8_t> SolveGf3(const std::vector<uint64_t> &hashes, uint32_t layer, uint64_t cells) {
		uint64_t rows = hashes.size();
		uint64_t words = (cells + 63) / 64;
		uint64_t stride = 2 * words;
		std::vector<uint64_t> matrix(rows * stride, 0);
		std::vector<uint8_t> trits(cells);
		for (uint64_t r = 0; r < rows; r++) {
			Gf3Row(LayerSeed(hashes[r], layer), cells, trits.data());
			uint64_t *one = &matrix[r * stride], *two = one + words;
			for (uint64_t i = 0; i < cells; i++) {
				if (trits[i] == 1) {
					one[i >> 6] |= 1ULL << (i & 63);
				} else if (trits[i] == 2) {
					two[i >> 6] |= 1ULL << (i & 63);
				}
			}
		}
		std::vector<int64_t> pivot_col(rows, -1);
		std::vector<bool> is_pivot(cells, false);
		uint64_t rank = 0;
		for (uint64_t col = 0; col < cells && rank < rows; col++) {
			uint64_t w = col >> 6, bit = 1ULL << (col & 63);
			uint64_t found = rows;
			for (uint64_t r = rank; r < rows; r++) {
				const uint64_t *row = &matrix[r * stride];
				if ((row[w] | row[words + w]) & bit) {
					found = r;
					break;
				}
			}
			if (found == rows) {
				continue;
			}
			if (found != rank) {
				std::swap_ranges(&matrix[found * stride], &matrix[found * stride] + stride, &matrix[rank * stride]);
			}
			uint64_t *pivot = &matrix[rank * stride];
			if (pivot[words + w] & bit) { // normalise: multiplying by 2 swaps the planes
				std::swap_ranges(pivot, pivot + words, pivot + words);
			}
			const uint64_t *p1 = pivot, *p2 = pivot + words;
			for (uint64_t r = 0; r < rows; r++) {
				if (r == rank) {
					continue;
				}
				uint64_t *r1 = &matrix[r * stride], *r2 = r1 + words;
				// row -= factor * pivot: add the pivot times 2 (factor 1) or times 1 (factor 2)
				const uint64_t *a1, *a2;
				if (r1[w] & bit) {
					a1 = p2;
					a2 = p1;
				} else if (r2[w] & bit) {
					a1 = p1;
					a2 = p2;
				} else {
					continue;
				}
				for (uint64_t i = w; i < words; i++) {
					uint64_t x1 = r1[i], x2 = r2[i], y1 = a1[i], y2 = a2[i];
					uint64_t x0 = ~(x1 | x2), y0 = ~(y1 | y2);
					r1[i] = (x1 & y0) | (x0 & y1) | (x2 & y2); // 1+0, 0+1, 2+2
					r2[i] = (x2 & y0) | (x0 & y2) | (x1 & y1); // 2+0, 0+2, 1+1
				}
			}
			pivot_col[rank] = (int64_t)col;
			is_pivot[col] = true;
			rank++;
		}
		std::vector<uint8_t> v(cells, 0);
		uint64_t seed = Mix64(hashes.empty() ? 7 : hashes[0] + layer + 54321);
		bool any = false;
		int64_t first_free = -1;
		for (uint64_t col = 0; col < cells; col++) {
			if (is_pivot[col]) {
				continue;
			}
			if (first_free < 0) {
				first_free = (int64_t)col;
			}
			seed = Mix64(seed + col);
			v[col] = (uint8_t)MulHi(seed, 3);
			any = any || v[col] != 0;
		}
		if (!any) {
			v[first_free] = 1;
		}
		// v restricted to the free cells, bit-sliced
		std::vector<uint64_t> v1(words, 0), v2(words, 0);
		for (uint64_t col = 0; col < cells; col++) {
			if (v[col] == 1) {
				v1[col >> 6] |= 1ULL << (col & 63);
			} else if (v[col] == 2) {
				v2[col >> 6] |= 1ULL << (col & 63);
			}
		}
		for (uint64_t r = 0; r < rank; r++) {
			const uint64_t *r1 = &matrix[r * stride], *r2 = r1 + words;
			// sum of row * v over the free cells (pivot cells of v are still zero)
			uint64_t ones = 0, twos = 0; // products equal to 1 and to 2
			for (uint64_t i = 0; i < words; i++) {
				ones += __builtin_popcountll((r1[i] & v1[i]) | (r2[i] & v2[i]));
				twos += __builtin_popcountll((r1[i] & v2[i]) | (r2[i] & v1[i]));
			}
			uint64_t sum = (ones + 2 * twos) % 3;
			v[(uint64_t)pivot_col[r]] = (uint8_t)((3 - sum) % 3);
		}
		return v;
	}

	static void WriteTrits(BitWriter &out, const std::vector<uint8_t> &trits) {
		uint64_t n = trits.size();
		for (uint64_t start = 0; start < n; start += TRIT_CHUNK) {
			uint64_t count = std::min<uint64_t>(TRIT_CHUNK, n - start);
			unsigned __int128 value = 0;
			for (uint64_t i = count; i-- > 0;) {
				value = value * 3 + trits[start + i];
			}
			uint64_t bits = TritBits(count);
			if (bits > 64) {
				out.Write((uint64_t)(value >> 64), (uint32_t)(bits - 64));
				out.Write((uint64_t)value, 64);
			} else {
				out.Write((uint64_t)value, (uint32_t)bits);
			}
		}
	}

	// dot product of the stored trits with a pseudo-random GF(3) row, mod 3
	static uint32_t DotTrits(BitReader &in, uint64_t cells, const uint8_t *row) {
		uint64_t sum = 0;
		static const unsigned __int128 HALF = []() {
			unsigned __int128 p = 1;
			for (int i = 0; i < 40; i++) {
				p *= 3;
			}
			return p;
		}();
		for (uint64_t start = 0; start < cells; start += TRIT_CHUNK) {
			uint64_t count = std::min<uint64_t>(TRIT_CHUNK, cells - start);
			uint64_t bits = TritBits(count);
			unsigned __int128 value;
			if (bits > 64) {
				uint64_t high = in.Read((uint32_t)(bits - 64));
				uint64_t low = in.Read(64);
				value = ((unsigned __int128)high << 64) | low;
			} else {
				value = in.Read((uint32_t)bits);
			}
			uint64_t parts[2] = {(uint64_t)(value % HALF), (uint64_t)(value / HALF)};
			for (uint64_t i = 0; i < count; i++) {
				uint64_t &part = parts[i / 40];
				uint64_t q = part / 3;
				sum += (part - q * 3) * row[start + i];
				part = q;
			}
		}
		return (uint32_t)(sum % 3);
	}

	struct Plan {
		uint64_t keys = 0;
		uint32_t bucket_code = 0;
		uint64_t buckets = 0;
		std::vector<std::vector<uint64_t>> bucket_hashes;
		std::vector<Mode> modes; // per bucket
		uint64_t bits = 0;
		double expected_eps = 1.0; // average over buckets (queries hit buckets uniformly)
	};

	static void WriteCounts(BitWriter &out, const Plan &plan, uint32_t rice) {
		uint64_t base = plan.keys / plan.buckets;
		for (uint64_t b = 0; b + 1 < plan.buckets; b++) {
			int64_t delta = (int64_t)plan.bucket_hashes[b].size() - (int64_t)base;
			uint64_t zigzag = delta >= 0 ? (uint64_t)delta * 2 : (uint64_t)(-delta) * 2 - 1;
			out.WriteOnes(zigzag >> rice);
			out.Write(0, 1);
			out.Write(zigzag, rice);
		}
	}
	static uint64_t CountBits(const Plan &plan, uint32_t &best_rice) {
		uint64_t base = plan.keys / plan.buckets;
		uint64_t best = ~0ULL;
		for (uint32_t rice = 0; rice < 16; rice++) {
			uint64_t bits = 0;
			for (uint64_t b = 0; b + 1 < plan.buckets; b++) {
				int64_t delta = (int64_t)plan.bucket_hashes[b].size() - (int64_t)base;
				uint64_t zigzag = delta >= 0 ? (uint64_t)delta * 2 : (uint64_t)(-delta) * 2 - 1;
				bits += (zigzag >> rice) + 1 + rice;
			}
			if (bits < best) {
				best = bits;
				best_rice = rice;
			}
		}
		return best;
	}

	// Distinct hashes in, plan with a mode per bucket that minimises the expected false-positive rate
	// within budget_bits (header included).
	static Plan MakePlan(const std::vector<uint64_t> &hashes, uint64_t budget_bits) {
		Plan plan;
		plan.keys = hashes.size();
		plan.bucket_code = BucketCode(plan.keys, budget_bits);
		plan.buckets = BucketCount(plan.keys, plan.bucket_code);
		plan.bucket_hashes.resize(plan.buckets);
		for (auto h : hashes) {
			plan.bucket_hashes[BucketOf(h, plan.buckets)].push_back(h);
		}
		uint32_t rice = 0;
		uint64_t header = 7 + BitLength(plan.keys) + BUCKET_CODE_BITS + 4 + CountBits(plan, rice);
		std::vector<Mode> all_modes;
		for (uint32_t gf3 = 0; gf3 <= MAX_GF3; gf3++) {
			for (uint32_t gf2 = 0; gf2 <= MAX_GF2; gf2++) {
				all_modes.push_back(Mode {gf2, gf3});
			}
		}
		// mode table: up to 4 distinct modes (7 bits each), then 2 bits per bucket
		uint64_t mode_header = 3 + 4 * 7 + 2 * plan.buckets;
		if (header + mode_header >= budget_bits) {
			plan.bits = ~0ULL;
			return plan;
		}
		uint64_t avail = budget_bits - header - mode_header;
		uint64_t nb = plan.buckets;
		// Lagrangian relaxation: per bucket, minimise eps + lambda * bits
		auto choose = [&](double lambda, std::vector<uint32_t> &choice) {
			uint64_t total = 0;
			for (uint64_t b = 0; b < nb; b++) {
				uint64_t cells = plan.bucket_hashes[b].size() + 1;
				if (cells == 1) {
					choice[b] = 0;
					continue;
				}
				double best = 1e300;
				for (uint32_t i = 0; i < all_modes.size(); i++) {
					double value = ModeEps(all_modes[i]) + lambda * ModeBits(all_modes[i], cells);
					if (value < best) {
						best = value;
						choice[b] = i;
					}
				}
				total += ModeBits(all_modes[choice[b]], cells);
			}
			return total;
		};
		std::vector<uint32_t> choice(nb, 0), trial(nb, 0);
		double lo = 0, hi = 1;
		for (int iter = 0; iter < 100; iter++) {
			double mid = (lo + hi) / 2;
			if (choose(mid, trial) <= avail) {
				hi = mid;
				choice = trial;
			} else {
				lo = mid;
			}
		}
		choose(hi, choice);
		uint64_t used = 0;
		for (uint64_t b = 0; b < nb; b++) {
			used += ModeBits(all_modes[choice[b]], plan.bucket_hashes[b].size() + 1);
		}
		// greedy: spend the leftover bits on the best eps reduction per bit
		while (true) {
			double best_gain = 0;
			uint64_t best_bucket = nb;
			uint32_t best_mode = 0;
			for (uint64_t b = 0; b < nb; b++) {
				uint64_t cells = plan.bucket_hashes[b].size() + 1;
				if (cells == 1) {
					continue;
				}
				uint64_t cur_bits = ModeBits(all_modes[choice[b]], cells);
				double cur_eps = ModeEps(all_modes[choice[b]]);
				for (uint32_t i = 0; i < all_modes.size(); i++) {
					uint64_t bits = ModeBits(all_modes[i], cells);
					double eps = ModeEps(all_modes[i]);
					if (eps >= cur_eps || bits <= cur_bits || used - cur_bits + bits > avail) {
						continue;
					}
					double gain = (cur_eps - eps) / (double)(bits - cur_bits);
					if (gain > best_gain) {
						best_gain = gain;
						best_bucket = b;
						best_mode = i;
					}
				}
			}
			if (best_bucket == nb) {
				break;
			}
			used += ModeBits(all_modes[best_mode], plan.bucket_hashes[best_bucket].size() + 1) -
			        ModeBits(all_modes[choice[best_bucket]], plan.bucket_hashes[best_bucket].size() + 1);
			choice[best_bucket] = best_mode;
		}
		// keep at most 4 distinct modes: map rarely used modes down to a cheaper used one
		std::vector<uint32_t> distinct(choice.begin(), choice.end());
		std::sort(distinct.begin(), distinct.end());
		distinct.erase(std::unique(distinct.begin(), distinct.end()), distinct.end());
		while (distinct.size() > 4) {
			// drop the mode used by the fewest buckets, moving them to the next cheaper kept mode. Mode 0 (no
			// layers) is never dropped: it is the fallback, so dropping it would re-add it forever. Every
			// round removes one nonzero mode, so this terminates.
			uint32_t drop = 0;
			uint64_t drop_count = ~0ULL;
			for (auto mode : distinct) {
				if (mode == 0) {
					continue;
				}
				uint64_t count = std::count(choice.begin(), choice.end(), mode);
				if (count < drop_count) {
					drop_count = count;
					drop = mode;
				}
			}
			distinct.erase(std::find(distinct.begin(), distinct.end(), drop));
			for (uint64_t b = 0; b < nb; b++) {
				if (choice[b] != drop) {
					continue;
				}
				uint64_t cells = plan.bucket_hashes[b].size() + 1;
				uint64_t drop_bits = ModeBits(all_modes[drop], cells);
				uint32_t replacement = 0; // mode (0, 0) costs nothing
				double replacement_eps = 2;
				for (auto mode : distinct) {
					if (ModeBits(all_modes[mode], cells) <= drop_bits && ModeEps(all_modes[mode]) < replacement_eps) {
						replacement = mode;
						replacement_eps = ModeEps(all_modes[mode]);
					}
				}
				choice[b] = replacement;
			}
			if (std::find(distinct.begin(), distinct.end(), 0u) == distinct.end() &&
			    std::find(choice.begin(), choice.end(), 0u) != choice.end()) {
				distinct.push_back(0);
				std::sort(distinct.begin(), distinct.end());
			}
		}
		plan.modes.resize(nb);
		double eps_sum = 0;
		uint64_t vector_bits = 0;
		for (uint64_t b = 0; b < nb; b++) {
			plan.modes[b] = all_modes[choice[b]];
			uint64_t cells = plan.bucket_hashes[b].size() + 1;
			if (cells > 1) {
				eps_sum += ModeEps(plan.modes[b]);
				vector_bits += ModeBits(plan.modes[b], cells);
			}
		}
		plan.expected_eps = eps_sum / nb;
		plan.bits = header + mode_header + vector_bits;
		return plan;
	}

	// Layout: key count (var), bucket size code (2 bits), count rice parameter (4 bits), bucket counts but the last
	// (rice-coded zigzag deltas from keys / buckets), mode table size - 1 (3 bits), modes (5 + 2 bits each), mode index
	// per bucket (2 bits), then per bucket the GF(2) vectors and the GF(3) vectors.
	static void Write(BitWriter &out, const Plan &plan) {
		uint32_t rice = 0;
		CountBits(plan, rice);
		out.WriteVar(plan.keys);
		out.Write(plan.bucket_code, BUCKET_CODE_BITS);
		out.Write(rice, 4);
		WriteCounts(out, plan, rice);
		std::vector<std::pair<uint32_t, uint32_t>> table;
		for (auto &mode : plan.modes) {
			std::pair<uint32_t, uint32_t> key(mode.gf2, mode.gf3);
			if (std::find(table.begin(), table.end(), key) == table.end()) {
				table.push_back(key);
			}
		}
		out.Write(table.size() - 1, 3);
		for (auto &entry : table) {
			out.Write(entry.first, 5);
			out.Write(entry.second, 2);
		}
		for (auto &mode : plan.modes) {
			std::pair<uint32_t, uint32_t> key(mode.gf2, mode.gf3);
			out.Write(std::find(table.begin(), table.end(), key) - table.begin(), 2);
		}
		for (uint64_t b = 0; b < plan.buckets; b++) {
			const auto &hashes = plan.bucket_hashes[b];
			uint64_t cells = hashes.size() + 1;
			if (cells == 1) {
				continue;
			}
			for (uint32_t layer = 0; layer < plan.modes[b].gf2; layer++) {
				auto v = SolveGf2(hashes, layer, cells);
				for (uint64_t i = 0; i < cells; i += 64) {
					out.Write(v[i >> 6], (uint32_t)std::min<uint64_t>(64, cells - i));
				}
			}
			for (uint32_t layer = 0; layer < plan.modes[b].gf3; layer++) {
				auto v = SolveGf3(hashes, MAX_GF2 + 1 + layer, cells);
				WriteTrits(out, v);
			}
		}
	}

	static bool MayContain(BitReader in, uint64_t hash) {
		uint64_t keys = in.ReadVar();
		uint32_t bucket_code = (uint32_t)in.Read(BUCKET_CODE_BITS);
		uint32_t rice = (uint32_t)in.Read(4);
		uint64_t buckets = BucketCount(keys, bucket_code);
		uint64_t base = keys / buckets;
		uint64_t target = BucketOf(hash, buckets);
		// bucket sizes
		uint64_t remaining = keys;
		uint64_t counts_target = 0;
		std::vector<uint64_t> counts(buckets);
		for (uint64_t b = 0; b + 1 < buckets; b++) {
			uint64_t zigzag = (in.ReadUnary() << rice) | in.Read(rice);
			int64_t delta = (zigzag & 1) ? -(int64_t)((zigzag + 1) / 2) : (int64_t)(zigzag / 2);
			counts[b] = (uint64_t)((int64_t)base + delta);
			remaining -= counts[b];
		}
		counts[buckets - 1] = remaining;
		counts_target = counts[target];
		if (counts_target == 0) {
			return false;
		}
		uint32_t table_size = (uint32_t)in.Read(3) + 1;
		Mode table[8];
		for (uint32_t i = 0; i < table_size; i++) {
			table[i].gf2 = (uint32_t)in.Read(5);
			table[i].gf3 = (uint32_t)in.Read(2);
		}
		uint64_t offset = 0;
		Mode target_mode {0, 0};
		uint64_t mode_start = in.pos;
		for (uint64_t b = 0; b < buckets; b++) {
			const Mode &mode = table[in.Read(2)];
			if (b < target && counts[b] > 0) {
				offset += ModeBits(mode, counts[b] + 1);
			}
			if (b == target) {
				target_mode = mode;
			}
		}
		in.pos = mode_start + 2 * buckets + offset;
		uint64_t cells = counts_target + 1;
		uint64_t row[64];
		if (cells > 64 * 64) {
			return true; // cannot happen with buckets of ~256 keys; be safe
		}
		for (uint32_t layer = 0; layer < target_mode.gf2; layer++) {
			Gf2Row(LayerSeed(hash, layer), cells, row);
			uint64_t parity = 0;
			for (uint64_t i = 0; i < cells; i += 64) {
				uint32_t count = (uint32_t)std::min<uint64_t>(64, cells - i);
				parity ^= in.Read(count) & row[i >> 6];
			}
			if (__builtin_popcountll(parity) & 1) {
				return false;
			}
		}
		if (target_mode.gf3 > 0) {
			std::vector<uint8_t> trits(cells);
			for (uint32_t layer = 0; layer < target_mode.gf3; layer++) {
				Gf3Row(LayerSeed(hash, MAX_GF2 + 1 + layer), cells, trits.data());
				if (DotTrits(in, cells, trits.data()) != 0) {
					return false;
				}
			}
		}
		return true;
	}
};

} // namespace pruning_codec
} // namespace duckdb
