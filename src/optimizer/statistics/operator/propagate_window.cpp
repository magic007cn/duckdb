#include "duckdb/optimizer/statistics_propagator.hpp"
#include "duckdb/planner/expression/bound_window_expression.hpp"
#include "duckdb/planner/operator/logical_window.hpp"

namespace duckdb {

unique_ptr<NodeStatistics> StatisticsPropagator::PropagateStatistics(LogicalWindow &window,
                                                                     unique_ptr<LogicalOperator> *node_ptr) {
	// first propagate to the child
	node_stats = PropagateStatistics(window.children[0]);

	if (window.children[0]->type == LogicalOperatorType::LOGICAL_EMPTY_RESULT) {
//		ReplaceWithEmptyResult(*node_ptr);
//		return std::move(node_stats);
	}

	// then propagate to each of the order expressions
	for (auto &window_expr : window.expressions) {
		auto over_expr = reinterpret_cast<BoundWindowExpression *>(window_expr.get());
		for (auto &expr : over_expr->partitions) {
			over_expr->partitions_stats.push_back(PropagateExpression(expr));
		}
		for (auto &bound_order : over_expr->orders) {
			bound_order.stats = PropagateExpression(bound_order.expression);
		}
	}
	D_ASSERT(node_ptr->get()->GetColumnBindings() == window.GetColumnBindings());
	return std::move(node_stats);
}

} // namespace duckdb
