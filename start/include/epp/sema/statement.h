#ifndef EPP_INCLUDE_EPP_SEMA_STATEMENT_H_
#define EPP_INCLUDE_EPP_SEMA_STATEMENT_H_

#include <optional>

#include "epp/ast/node.h"
#include "epp/sema/error.h"
#include "epp/sema/function_table.h"
#include "epp/sema/scope.h"
#include "epp/sema/type.h"

namespace epp::sema {

// 文の意味検査の結果。成功はnullopt、失敗は最初のエラー。
using StatementAnalysisResult = CheckResult;

// 宣言、代入、呼び出し、戻り値、分岐とループを検査する。
// ブロックごとにスコープを作り、到達不能な文も型や名前の検査対象にする。
// return_typeがnulloptなら、値のないliveruだけを認める。
[[nodiscard]] StatementAnalysisResult AnalyzeStatement(
  ast::NodeId statement_id,
  const ast::Ast& ast,
  ScopeStack& scope_stack,
  const FunctionTable& function_table,
  std::optional<BuiltInType> return_type
);

} // namespace epp::sema

#endif // EPP_INCLUDE_EPP_SEMA_STATEMENT_H_
