#include "duckdb/util/util.hpp"

namespace duckdb {
uint64_t Util::session_id;
uint64_t Util::command_count;
thread_local uint64_t Util::stat_eval_time = 0;
thread_local bool Util::track_stat_eval_time = false;
thread_local uint64_t Util::actual_rows_scanned = 0;
thread_local bool Util::track_actual_rows_scanned = false;
thread_local vector<StatisticSizeInfo> Util::statistic_sizes;
thread_local bool Util::track_statistic_sizes = false;
bool Util::log_statistics;
} // namespace duckdb
