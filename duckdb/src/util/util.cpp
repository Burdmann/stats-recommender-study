#include "duckdb/util/util.hpp"

namespace duckdb {
uint64_t Util::session_id;
uint64_t Util::command_count;
thread_local uint64_t Util::stat_eval_time = 0;
thread_local bool Util::track_stat_eval_time = false;
} // namespace duckdb
