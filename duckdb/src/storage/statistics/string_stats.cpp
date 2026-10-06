#include "duckdb/storage/statistics/string_stats.hpp"

#include "duckdb/common/serializer/deserializer.hpp"
#include "duckdb/common/serializer/serializer.hpp"
#include "duckdb/common/string_util.hpp"
#include "duckdb/common/types/vector.hpp"
#include "duckdb/main/error_manager.hpp"
#include "duckdb/storage/statistics/base_statistics.hpp"
#include "utf8proc_wrapper.hpp"

namespace duckdb {

BaseStatistics StringStats::CreateUnknown(LogicalType type) {
	BaseStatistics result(std::move(type));
	result.InitializeUnknown();
	auto &string_data = StringStats::GetDataUnsafe(result);
	string_data.max_string_length = 0;
	string_data.has_max_string_length = false;
	string_data.has_unicode = true;
	return result;
}

BaseStatistics StringStats::CreateEmpty(LogicalType type) {
	BaseStatistics result(std::move(type));
	result.InitializeEmpty();
	auto &string_data = StringStats::GetDataUnsafe(result);
	string_data.max_string_length = 0;
	string_data.has_max_string_length = true;
	string_data.has_unicode = false;
	return result;
}

StringStatsData &StringStats::GetDataUnsafe(BaseStatistics &stats) {
	D_ASSERT(stats.GetStatsType() == StatisticsType::STRING_STATS);
	return stats.stats_union.string_data;
}

const StringStatsData &StringStats::GetDataUnsafe(const BaseStatistics &stats) {
	D_ASSERT(stats.GetStatsType() == StatisticsType::STRING_STATS);
	return stats.stats_union.string_data;
}

bool StringStats::HasMaxStringLength(const BaseStatistics &stats) {
	if (stats.GetType().id() == LogicalTypeId::SQLNULL) {
		return false;
	}
	return GetDataUnsafe(stats).has_max_string_length;
}

bool StringStats::HasMinMax(const BaseStatistics &) {
	return false;
}

uint32_t StringStats::MaxStringLength(const BaseStatistics &stats) {
	if (!HasMaxStringLength(stats)) {
		throw InternalException("MaxStringLength called on statistics that does not have a max string length");
	}
	return StringStats::GetDataUnsafe(stats).max_string_length;
}

bool StringStats::CanContainUnicode(const BaseStatistics &stats) {
	if (stats.GetType().id() == LogicalTypeId::SQLNULL) {
		return true;
	}
	return GetDataUnsafe(stats).has_unicode;
}

string StringStats::Min(const BaseStatistics &) {
	throw InternalException("Min() called on string statistics without min/max values");
}

string StringStats::Max(const BaseStatistics &) {
	throw InternalException("Max() called on string statistics without min/max values");
}

void StringStats::ResetMaxStringLength(BaseStatistics &stats) {
	StringStats::GetDataUnsafe(stats).has_max_string_length = false;
}

void StringStats::SetMaxStringLength(BaseStatistics &stats, uint32_t length) {
	auto &data = StringStats::GetDataUnsafe(stats);
	data.has_max_string_length = true;
	data.max_string_length = length;
}

void StringStats::SetContainsUnicode(BaseStatistics &stats) {
	StringStats::GetDataUnsafe(stats).has_unicode = true;
}

void StringStats::Serialize(const BaseStatistics &stats, Serializer &serializer) {
	auto &string_data = StringStats::GetDataUnsafe(stats);
	data_t unavailable_min[StringStatsData::MAX_STRING_MINMAX_SIZE] = {};
	data_t unavailable_max[StringStatsData::MAX_STRING_MINMAX_SIZE] = {};
	serializer.WriteProperty(200, "min", unavailable_min, StringStatsData::MAX_STRING_MINMAX_SIZE);
	serializer.WriteProperty(201, "max", unavailable_max, StringStatsData::MAX_STRING_MINMAX_SIZE);
	serializer.WriteProperty(202, "has_unicode", string_data.has_unicode);
	serializer.WriteProperty(203, "has_max_string_length", string_data.has_max_string_length);
	serializer.WriteProperty(204, "max_string_length", string_data.max_string_length);
}

void StringStats::Deserialize(Deserializer &deserializer, BaseStatistics &base) {
	auto &string_data = StringStats::GetDataUnsafe(base);
	data_t ignored_min[StringStatsData::MAX_STRING_MINMAX_SIZE];
	data_t ignored_max[StringStatsData::MAX_STRING_MINMAX_SIZE];
	deserializer.ReadProperty(200, "min", ignored_min, StringStatsData::MAX_STRING_MINMAX_SIZE);
	deserializer.ReadProperty(201, "max", ignored_max, StringStatsData::MAX_STRING_MINMAX_SIZE);
	deserializer.ReadProperty(202, "has_unicode", string_data.has_unicode);
	deserializer.ReadProperty(203, "has_max_string_length", string_data.has_max_string_length);
	deserializer.ReadProperty(204, "max_string_length", string_data.max_string_length);
}

static int StringValueComparison(const_data_ptr_t data, idx_t len, const_data_ptr_t comparison) {
	for (idx_t i = 0; i < len; i++) {
		if (data[i] < comparison[i]) {
			return -1;
		} else if (data[i] > comparison[i]) {
			return 1;
		}
	}
	return 0;
}

void StringStats::Update(BaseStatistics &stats, const string_t &value) {
	auto data = const_data_ptr_cast(value.GetData());
	auto size = value.GetSize();
	auto &string_data = StringStats::GetDataUnsafe(stats);
	if (size > string_data.max_string_length) {
		string_data.max_string_length = UnsafeNumericCast<uint32_t>(size);
	}
	if (stats.GetType().id() == LogicalTypeId::VARCHAR && !string_data.has_unicode) {
		auto unicode = Utf8Proc::Analyze(const_char_ptr_cast(data), size);
		if (unicode == UnicodeType::UTF8) {
			string_data.has_unicode = true;
		} else if (unicode == UnicodeType::INVALID) {
			throw ErrorManager::InvalidUnicodeError(string(const_char_ptr_cast(data), size),
			                                        "segment statistics update");
		}
	}
}

void StringStats::SetMin(BaseStatistics &, const string_t &) {
}

void StringStats::SetMax(BaseStatistics &, const string_t &) {
}

void StringStats::Merge(BaseStatistics &stats, const BaseStatistics &other) {
	if (other.GetType().id() == LogicalTypeId::VALIDITY) {
		return;
	}
	if (other.GetType().id() == LogicalTypeId::SQLNULL) {
		return;
	}
	auto &string_data = StringStats::GetDataUnsafe(stats);
	auto &other_data = StringStats::GetDataUnsafe(other);
	string_data.has_unicode = string_data.has_unicode || other_data.has_unicode;
	string_data.has_max_string_length = string_data.has_max_string_length && other_data.has_max_string_length;
	string_data.max_string_length = MaxValue<uint32_t>(string_data.max_string_length, other_data.max_string_length);
}

FilterPropagateResult StringStats::CheckZonemap(const BaseStatistics &, ExpressionType,
                                                array_ptr<const Value>) {
	return FilterPropagateResult::NO_PRUNING_POSSIBLE;
}

FilterPropagateResult StringStats::CheckZonemap(const_data_ptr_t min_data, idx_t min_len, const_data_ptr_t max_data,
                                                idx_t max_len, ExpressionType comparison_type, const string &constant) {
	auto data = const_data_ptr_cast(constant.c_str());
	idx_t size = constant.size();

	int min_comp = StringValueComparison(data, MinValue(min_len, size), min_data);
	int max_comp = StringValueComparison(data, MinValue(max_len, size), max_data);
	switch (comparison_type) {
	case ExpressionType::COMPARE_EQUAL:
	case ExpressionType::COMPARE_NOT_DISTINCT_FROM:
		if (min_comp >= 0 && max_comp <= 0) {
			return FilterPropagateResult::NO_PRUNING_POSSIBLE;
		} else {
			return FilterPropagateResult::FILTER_ALWAYS_FALSE;
		}
	case ExpressionType::COMPARE_NOTEQUAL:
	case ExpressionType::COMPARE_DISTINCT_FROM:
		if (min_comp < 0 || max_comp > 0) {
			return FilterPropagateResult::FILTER_ALWAYS_TRUE;
		}
		return FilterPropagateResult::NO_PRUNING_POSSIBLE;
	case ExpressionType::COMPARE_GREATERTHANOREQUALTO:
	case ExpressionType::COMPARE_GREATERTHAN:
		if (max_comp <= 0) {
			return FilterPropagateResult::NO_PRUNING_POSSIBLE;
		} else {
			return FilterPropagateResult::FILTER_ALWAYS_FALSE;
		}
	case ExpressionType::COMPARE_LESSTHAN:
	case ExpressionType::COMPARE_LESSTHANOREQUALTO:
		if (min_comp >= 0) {
			return FilterPropagateResult::NO_PRUNING_POSSIBLE;
		} else {
			return FilterPropagateResult::FILTER_ALWAYS_FALSE;
		}
	default:
		throw InternalException("Expression type not implemented for string statistics zone map");
	}
}

string StringStats::ToString(const BaseStatistics &stats) {
	auto &string_data = StringStats::GetDataUnsafe(stats);
	return StringUtil::Format("[Min: NULL, Max: NULL, Has Unicode: %s, Max String Length: %s]",
	                          string_data.has_unicode ? "true" : "false",
	                          string_data.has_max_string_length ? to_string(string_data.max_string_length) : "?");
}

void StringStats::Verify(const BaseStatistics &stats, Vector &vector, const SelectionVector &sel, idx_t count) {
	if (!HasMaxStringLength(stats)) {
		return;
	}
	auto &string_data = StringStats::GetDataUnsafe(stats);

	UnifiedVectorFormat vdata;
	vector.ToUnifiedFormat(count, vdata);
	auto data = UnifiedVectorFormat::GetData<string_t>(vdata);
	for (idx_t i = 0; i < count; i++) {
		auto idx = sel.get_index(i);
		auto index = vdata.sel->get_index(idx);
		if (!vdata.validity.RowIsValid(index)) {
			continue;
		}
		auto value = data[index];
		auto data = value.GetData();
		auto len = value.GetSize();
		// LCOV_EXCL_START
		if (string_data.has_max_string_length && len > string_data.max_string_length) {
			throw InternalException(
			    "Statistics mismatch: string value exceeds maximum string length.\nStatistics: %s\nVector: %s",
			    stats.ToString(), vector.ToString(count));
		}
		if (stats.GetType().id() == LogicalTypeId::VARCHAR && !string_data.has_unicode) {
			auto unicode = Utf8Proc::Analyze(data, len);
			if (unicode == UnicodeType::UTF8) {
				throw InternalException("Statistics mismatch: string value contains unicode, but statistics says it "
				                        "shouldn't.\nStatistics: %s\nVector: %s",
				                        stats.ToString(), vector.ToString(count));
			} else if (unicode == UnicodeType::INVALID) {
				throw InternalException("Invalid unicode detected in vector: %s", vector.ToString(count));
			}
		}
		// LCOV_EXCL_STOP
	}
}

} // namespace duckdb
