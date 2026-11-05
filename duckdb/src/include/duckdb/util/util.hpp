//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/util/util.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once
#include "duckdb/common/types.hpp"
// #include <ctime>
#include <chrono>

namespace duckdb {
struct StatisticSizeInfo {
	idx_t partition_index;
	idx_t column_index;
	idx_t size_bytes;
};

class Util {
public:
	static uint64_t session_id;
	static uint64_t command_count;
	static thread_local uint64_t stat_eval_time;
	static thread_local bool track_stat_eval_time;
	static thread_local uint64_t actual_rows_scanned;
	static thread_local bool track_actual_rows_scanned;
	static thread_local vector<StatisticSizeInfo> statistic_sizes;
	static thread_local bool track_statistic_sizes;

	static bool log_statistics;

	static inline uint64_t GetTime() {
		auto duration = std::chrono::steady_clock::now().time_since_epoch();
		return std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
	}

	static inline uint64_t GetTimeNano() {
		auto duration = std::chrono::steady_clock::now().time_since_epoch();
		return std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();
	}
};

} // namespace duckdb
