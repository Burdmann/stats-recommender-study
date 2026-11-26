#include "duckdb/planner/filter/expression_filter.hpp"
#include "duckdb/planner/expression/bound_constant_expression.hpp"
#include "duckdb/planner/expression/bound_function_expression.hpp"
#include "duckdb/planner/expression_iterator.hpp"
#include "duckdb/planner/expression/bound_reference_expression.hpp"
#include "duckdb/common/types/data_chunk.hpp"
#include "duckdb/execution/expression_executor.hpp"
#include "duckdb/storage/statistics/additional/stats_set.hpp"
#include "duckdb/common/types/string_type.hpp"

namespace duckdb {

ExpressionFilter::ExpressionFilter(unique_ptr<Expression> expr_p)
    : TableFilter(TableFilterType::EXPRESSION_FILTER), expr(std::move(expr_p)) {
}

bool ExpressionFilter::EvaluateWithConstant(ClientContext &context, const Value &val) {
	ExpressionExecutor executor(context, *expr);
	return EvaluateWithConstant(executor, val);
}

bool ExpressionFilter::EvaluateWithConstant(ExpressionExecutor &executor, const Value &val) const {
	DataChunk input;
	input.data.emplace_back(val);
	input.SetCardinality(1);

	SelectionVector sel(1);

	idx_t count = executor.SelectExpression(input, sel);
	return count > 0;
}

FilterPropagateResult ExpressionFilter::CheckStatistics(BaseStatistics &stats) const {
	if (expr->GetExpressionClass() == ExpressionClass::BOUND_FUNCTION &&
	    stats.GetType().InternalType() == PhysicalType::VARCHAR && stats.additional_stats != nullptr) {
		auto &function = expr->Cast<BoundFunctionExpression>();
		auto &name = function.function.name;
		bool is_like = name == "~~";
		// LikeOptimizationRule rewrites wildcard-free LIKE patterns into these functions before pushdown
		bool is_substring = name == "prefix" || name == "suffix" || name == "contains";
		if ((is_like || is_substring) && function.children.size() == 2 &&
		    function.children[0]->GetExpressionClass() == ExpressionClass::BOUND_REF &&
		    function.children[1]->GetExpressionClass() == ExpressionClass::BOUND_CONSTANT) {
			auto &pattern = function.children[1]->Cast<BoundConstantExpression>().value;
			if (!pattern.IsNull()) {
				auto &additional_stats = *((AdditionalStats<std::string> *)stats.additional_stats);
				auto &pattern_str = StringValue::Get(pattern);
				auto result = is_like ? StatisticsSet::QueryLike(&additional_stats, pattern_str)
				                      : StatisticsSet::QueryContains(&additional_stats, pattern_str);
				if (result == FilterPropagateResult::FILTER_ALWAYS_FALSE && stats.CanHaveNull()) {
					return FilterPropagateResult::FILTER_FALSE_OR_NULL;
				}
				return result;
			}
		}
	}
	return FilterPropagateResult::NO_PRUNING_POSSIBLE;
}

string ExpressionFilter::ToString(const string &column_name) const {
	auto name_expr = make_uniq<BoundReferenceExpression>(column_name, LogicalType::INVALID, 0ULL);
	return ToExpression(*name_expr)->ToString();
}

void ExpressionFilter::ReplaceExpressionRecursive(unique_ptr<Expression> &expr, const Expression &column,
                                                  ExpressionType replace_type) {
	if (expr->type == replace_type) {
		expr = column.Copy();
		return;
	}
	ExpressionIterator::EnumerateChildren(
	    *expr, [&](unique_ptr<Expression> &child) { ReplaceExpressionRecursive(child, column, replace_type); });
}

unique_ptr<Expression> ExpressionFilter::ToExpression(const Expression &column) const {
	auto expr_copy = expr->Copy();
	ReplaceExpressionRecursive(expr_copy, column);
	return expr_copy;
}

bool ExpressionFilter::Equals(const TableFilter &other_p) const {
	if (!TableFilter::Equals(other_p)) {
		return false;
	}
	auto &other = other_p.Cast<ExpressionFilter>();
	return other.expr->Equals(*expr);
}

unique_ptr<TableFilter> ExpressionFilter::Copy() const {
	return make_uniq<ExpressionFilter>(expr->Copy());
}

} // namespace duckdb
