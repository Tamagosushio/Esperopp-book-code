#ifndef EPP_INCLUDE_EPP_SEMA_DECLARATION_H_
#define EPP_INCLUDE_EPP_SEMA_DECLARATION_H_

#include <string>
#include <variant>
#include <vector>

#include "epp/ast/node.h"
#include "epp/sema/error.h"
#include "epp/sema/function_table.h"
#include "epp/sema/scope.h"
#include "epp/sema/type.h"
#include "epp/source/source_range.h"

namespace epp::sema {

// 読み取った変数名、名前の範囲、型、定数指定。
struct AnalyzedDeclaration {
  std::u32string normalized_identifier;
  source::SourceRange name_range;
  BuiltInType type;
  bool is_constant;
};

// 成功時は宣言の情報、失敗時は要素の過不足や不正な名前を表すエラー。
using DeclarationAnalysisResult = std::variant<
  AnalyzedDeclaration,
  Error
>;

// 型名と変数名を一つずつ取り出し、konstantaの有無を調べる。
[[nodiscard]] DeclarationAnalysisResult AnalyzeDeclarationComponents(
  const std::vector<ast::DeclarationComponent>& components
);

// 成功時は登録した変数の番号、失敗時は宣言または初期化式のエラー。
using LocalDeclarationRegistrationResult = std::variant<
  VariableSymbolId,
  Error
>;

// 初期化式の型を検査してから、現在のスコープへ変数を登録する。
// 同一スコープでの名前の重複はエラーにする。
[[nodiscard]] LocalDeclarationRegistrationResult RegisterLocalDeclaration(
  const ast::LocalDeclarationStatement& declaration,
  const ast::Ast& ast,
  ScopeStack& scope_stack,
  const FunctionTable& function_table
);

} // namespace epp::sema

#endif // EPP_INCLUDE_EPP_SEMA_DECLARATION_H_
