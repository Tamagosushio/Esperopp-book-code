#ifndef EPP_INCLUDE_EPP_SEMA_FUNCTION_H_
#define EPP_INCLUDE_EPP_SEMA_FUNCTION_H_

#include <cstddef>
#include <optional>
#include <variant>
#include <vector>

#include "epp/ast/node.h"
#include "epp/sema/error.h"
#include "epp/sema/function_table.h"
#include "epp/sema/scope.h"
#include "epp/sema/type.h"

namespace epp::sema {

// 関数ヘッダから確定した戻り値の型と、関数名を持つ要素の位置。
// vakaではreturn_typeがnulloptになる。
struct AnalyzedFunctionHead {
  std::optional<BuiltInType> return_type;
  std::size_t name_index;
};

// 成功時はヘッダの情報、失敗時は型名や関数名のエラー。
using FunctionHeadAnalysisResult = std::variant<
  AnalyzedFunctionHead,
  Error
>;

// 戻り値の型名またはvakaと、-i形の関数名を語順によらず取り出す。
[[nodiscard]] FunctionHeadAnalysisResult AnalyzeFunctionHeadComponents(
  const std::vector<ast::FunctionHeadComponent>& components
);

// 定義順に並べた関数の番号、または登録時に検出したエラー。
using FunctionCollectionResult = std::variant<
  std::vector<FunctionSymbolId>,
  Error
>;

// 関数本体を解析する前に、全関数の名前と署名を表へ登録する。相互再帰を解決できる。
[[nodiscard]] FunctionCollectionResult RegisterCompilationUnitFunctions(
  const ast::CompilationUnit& compilation_unit,
  const ast::Ast& ast,
  FunctionTable& function_table
);

// 宣言順に並べた仮引数の変数番号、または仮引数のエラー。
using FunctionParameterRegistrationResult = std::variant<
  std::vector<VariableSymbolId>,
  Error
>;

// 仮引数の型と名前を調べ、現在のスコープへ登録する。語順とkonstantaも検査する。
[[nodiscard]] FunctionParameterRegistrationResult RegisterFunctionParameters(
  const ast::FunctionDefinition& function_definition,
  const ast::Ast& ast,
  ScopeStack& scope_stack
);

// 全関数の検査結果。成功はnullopt、失敗は最初のエラー。
using CompilationUnitAnalysisResult = CheckResult;

// function_idsを指定すると、成功時だけ定義順の解決済み番号を保存する。
[[nodiscard]] CompilationUnitAnalysisResult AnalyzeCompilationUnit(
  const ast::CompilationUnit& compilation_unit,
  const ast::Ast& ast,
  FunctionTable& function_table,
  std::vector<FunctionSymbolId>* function_ids = nullptr
);

// 実行開始関数ĉefiの番号、または開始関数のエラー。
using EntryPointAnalysisResult = std::variant<
  FunctionSymbolId,
  Error
>;

// ĉefiの存在、entjeraの戻り値、仮引数がないことを検査する。
[[nodiscard]] EntryPointAnalysisResult AnalyzeEntryPoint(
  const FunctionTable& function_table
);

} // namespace epp::sema

#endif // EPP_INCLUDE_EPP_SEMA_FUNCTION_H_
