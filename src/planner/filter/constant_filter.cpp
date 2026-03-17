#include "duckdb/planner/filter/constant_filter.hpp"
#include "duckdb/storage/statistics/base_statistics.hpp"
#include "duckdb/planner/expression/bound_comparison_expression.hpp"
#include "duckdb/planner/expression/bound_constant_expression.hpp"
#include "duckdb/common/value_operations/value_operations.hpp"
#include "duckdb/common/enum_util.hpp"
#include "duckdb/util/util.hpp"
#include "duckdb/storage/table/column_data.hpp"

#include <iostream>

namespace duckdb {

std::unordered_set<uint64_t> ConstantFilter::scanned_partitions;

ConstantFilter::ConstantFilter(ExpressionType comparison_type_p, Value constant_p)
    : TableFilter(TableFilterType::CONSTANT_COMPARISON), comparison_type(comparison_type_p),
      constant(std::move(constant_p)) {
	if (constant.IsNull()) {
		throw InternalException("ConstantFilter constant cannot be NULL - use IsNullFilter instead");
	}
}

bool ConstantFilter::Compare(const Value &value) const {
	switch (comparison_type) {
	case ExpressionType::COMPARE_EQUAL:
		return ValueOperations::Equals(value, constant);
	case ExpressionType::COMPARE_NOTEQUAL:
		return ValueOperations::NotEquals(value, constant);
	case ExpressionType::COMPARE_GREATERTHAN:
		return ValueOperations::GreaterThan(value, constant);
	case ExpressionType::COMPARE_GREATERTHANOREQUALTO:
		return ValueOperations::GreaterThanEquals(value, constant);
	case ExpressionType::COMPARE_LESSTHAN:
		return ValueOperations::LessThan(value, constant);
	case ExpressionType::COMPARE_LESSTHANOREQUALTO:
		return ValueOperations::LessThanEquals(value, constant);
	default:
		throw InternalException("unknown comparison type for ConstantFilter: " + EnumUtil::ToString(comparison_type));
	}
}

FilterPropagateResult ConstantFilter::CheckStatistics(BaseStatistics &stats) const {
	if (!stats.CanHaveNoNull()) {
		// no non-null values are possible: always false
		return FilterPropagateResult::FILTER_ALWAYS_FALSE;
	}
	FilterPropagateResult result;
	D_ASSERT(constant.type().id() == stats.GetType().id());
	switch (constant.type().InternalType()) {
	case PhysicalType::UINT8:
	case PhysicalType::UINT16:
	case PhysicalType::UINT32:
	case PhysicalType::UINT64:
	case PhysicalType::UINT128:
	case PhysicalType::INT8:
	case PhysicalType::INT16:
	case PhysicalType::INT32:
	case PhysicalType::INT64:
	case PhysicalType::INT128:
	case PhysicalType::FLOAT:
	case PhysicalType::DOUBLE:
		result = NumericStats::CheckZonemap(stats, comparison_type, array_ptr<const Value>(&constant, 1));
		break;
	case PhysicalType::VARCHAR:
		result = StringStats::CheckZonemap(stats, comparison_type, array_ptr<const Value>(&constant, 1));
		break;
	default:
		return FilterPropagateResult::NO_PRUNING_POSSIBLE;
	}
	if (result == FilterPropagateResult::FILTER_ALWAYS_TRUE) {
		// the numeric filter is always true, but the column can have NULL values
		// we can't prune the filter
		if (stats.CanHaveNull()) {
			return FilterPropagateResult::NO_PRUNING_POSSIBLE;
		}
	}
	FilterPropagateResult new_result = result;
	if (result == FilterPropagateResult::NO_PRUNING_POSSIBLE)
		new_result =
		    ColumnData::QueryAdditionalStats(stats, comparison_type, constant.type().InternalType(), &constant);

	// printf("%p: %d\n", &stats, (int)new_result);

	// if (stats.is_rowgroup && new_result != FilterPropagateResult::FILTER_ALWAYS_FALSE) {
	// 	scanned_partitions.insert(stats.id);
	// 	// std::cout << "CHECK " << stats.id << " min: " << stats.stats_union.numeric_data.min.value_.integer
	// 	//           << " max: " << stats.stats_union.numeric_data.max.value_.integer << " " << stats.type.ToString()
	// 	//           << " result: " << (int)result << " is_rowgroup: " << stats.is_rowgroup
	// 	//           << " constant: " << constant.GetValueUnsafe<int>() << std::endl;
	// 	// std::cout << "CHECK " << stats.id << " min: " << stats.stats_union.numeric_data.min.value_.ubigint
	// 	//           << " max: " << stats.stats_union.numeric_data.max.value_.ubigint << " " << stats.type.ToString()
	// 	//           << " result: " << (int)result << " is_rowgroup: " << stats.is_rowgroup
	// 	//           << " constant: " << constant.GetValueUnsafe<int64_t>() << std::endl;
	// }
	// std::cout << "CHECK " << stats.id << " min: " << stats.stats_union.numeric_data.min.value_.integer
	//           << " max: " << stats.stats_union.numeric_data.max.value_.integer << " " << stats.type.ToString()
	//           << " result before additional: " << (int)result << " result after additional: " << (int)new_result
	//           << " is_rowgroup: " << stats.is_rowgroup << " constant: " << constant.GetValueUnsafe<int>() <<
	//           std::endl;

	return new_result;
}

string ConstantFilter::ToString(const string &column_name) const {
	return column_name + ExpressionTypeToOperator(comparison_type) + constant.ToSQLString();
}

unique_ptr<Expression> ConstantFilter::ToExpression(const Expression &column) const {
	auto bound_constant = make_uniq<BoundConstantExpression>(constant);
	auto result = make_uniq<BoundComparisonExpression>(comparison_type, column.Copy(), std::move(bound_constant));
	return std::move(result);
}

bool ConstantFilter::Equals(const TableFilter &other_p) const {
	if (!TableFilter::Equals(other_p)) {
		return false;
	}
	auto &other = other_p.Cast<ConstantFilter>();
	return other.comparison_type == comparison_type && other.constant == constant;
}

unique_ptr<TableFilter> ConstantFilter::Copy() const {
	return make_uniq<ConstantFilter>(comparison_type, constant);
}

} // namespace duckdb
