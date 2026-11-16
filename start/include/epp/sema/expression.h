#ifndef EPP_INCLUDE_EPP_SEMA_EXPRESSION_H_
#define EPP_INCLUDE_EPP_SEMA_EXPRESSION_H_

#include <variant>

#include "epp/ast/node.h"
#include "epp/sema/error.h"
#include "epp/sema/function_table.h"
#include "epp/sema/scope.h"
#include "epp/sema/type.h"

namespace epp::sema {

// 成功時は呼び出す関数の番号、失敗時は名前や実引数のエラー。
using CallAnalysisResult = std::variant<
  FunctionSymbolId,
  Error
>;

// 値を持つ式の型、または意味解析エラー。
using ExpressionTypeResult = std::variant<BuiltInType, Error>;

// 呼び出し先の-u形、実引数の数と型を検査する。
// 値を返さない組み込み関数も対象。
[[nodiscard]] CallAnalysisResult AnalyzeCallExpression(
  ast::NodeId expression_id,
  const ast::Ast& ast,
  const ScopeStack& scope_stack,
  const FunctionTable& function_table
);

// 式の名前と型を検査する。
// 値を返さない呼び出しを式の値として使うとエラーにする。
[[nodiscard]] ExpressionTypeResult AnalyzeExpression(
  ast::NodeId expression_id,
  const ast::Ast& ast,
  const ScopeStack& scope_stack,
  const FunctionTable& function_table
);

} // namespace epp::sema

#endif // EPP_INCLUDE_EPP_SEMA_EXPRESSION_H_
