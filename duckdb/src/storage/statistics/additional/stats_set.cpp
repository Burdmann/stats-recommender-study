#include "duckdb/storage/statistics/additional/stats_set.hpp"

namespace duckdb {

STATISTIC_TYPE StatisticsSet::default_type = STATISTIC_TYPE::NONE;
std::unordered_map<std::tuple<std::string, unsigned int, unsigned int>, STATISTIC_TYPE, StatisticsSetKeyHash>
    StatisticsSet::mapping;

} // namespace duckdb
