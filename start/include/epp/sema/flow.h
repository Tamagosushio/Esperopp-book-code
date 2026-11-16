#ifndef EPP_INCLUDE_EPP_SEMA_FLOW_H_
#define EPP_INCLUDE_EPP_SEMA_FLOW_H_

#include <variant>

#include "epp/ast/node.h"
#include "epp/sema/error.h"

namespace epp::sema {

// 文から後続の文へ進めるか、rompuで現在のループを抜ける経路があるかを表す。
struct StatementFlow {
  bool can_complete_normally = false;
  bool can_break = false;
  [[nodiscard]] bool operator==(const StatementFlow& other) const noexcept = default;
};

// 制御の経路、または文の種類が不正な場合のエラー。
using FlowAnalysisResult = std::variant<StatementFlow, Error>;

// 分岐とループをたどり、関数末尾への到達やrompuの経路を調べる。
// 真理値リテラルの条件は確定できる。式の型検査は別の段階で行う。
[[nodiscard]] FlowAnalysisResult AnalyzeStatementFlow(
  ast::NodeId statement_id,
  const ast::Ast& ast
);

} // namespace epp::sema

#endif // EPP_INCLUDE_EPP_SEMA_FLOW_H_
