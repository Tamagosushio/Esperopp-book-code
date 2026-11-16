#ifndef EPP_INCLUDE_EPP_SEMA_NAME_RESOLUTION_H_
#define EPP_INCLUDE_EPP_SEMA_NAME_RESOLUTION_H_

#include <optional>
#include <string_view>
#include <variant>

#include "epp/ast/node.h"
#include "epp/sema/error.h"
#include "epp/sema/function_table.h"
#include "epp/sema/scope.h"

namespace epp::sema {

// 成功時は変数の番号、失敗時は名前式のエラー。
using VariableNameResolutionResult = std::variant<
  VariableSymbolId,
  Error
>;

// 名前式を内側のスコープから順に検索する。定数への参照も許す。
[[nodiscard]] VariableNameResolutionResult ResolveVariableName(
  ast::NodeId expression_id,
  const ast::Ast& ast,
  const ScopeStack& scope_stack
);

// 代入先の名前を解決する。未定義の変数とkonstantaへの代入はエラーにする。
[[nodiscard]] VariableNameResolutionResult ResolveMutableVariableName(
  ast::NodeId expression_id,
  const ast::Ast& ast,
  const ScopeStack& scope_stack
);

// -u形を定義名の-i形へ変換し、関数表から探す。形が不正か未定義ならnulloptを返す。
[[nodiscard]] std::optional<FunctionSymbolId> ResolveDirectCallFunction(
  std::u32string_view direct_call_identifier,
  const FunctionTable& function_table
);

// 成功時は呼び出し先の番号、失敗時は呼び出し先の名前やASTのエラー。
using CallTargetResolutionResult = std::variant<
  FunctionSymbolId,
  Error
>;

// 呼び出し式の名前を解決する。実引数の数と型はこの関数では調べない。
[[nodiscard]] CallTargetResolutionResult ResolveCallTarget(
  ast::NodeId expression_id,
  const ast::Ast& ast,
  const FunctionTable& function_table
);

} // namespace epp::sema

#endif // EPP_INCLUDE_EPP_SEMA_NAME_RESOLUTION_H_
