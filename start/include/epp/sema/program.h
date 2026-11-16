#ifndef EPP_INCLUDE_EPP_SEMA_PROGRAM_H_
#define EPP_INCLUDE_EPP_SEMA_PROGRAM_H_

#include <variant>

#include <vector>

#include "epp/ast/node.h"
#include "epp/sema/error.h"
#include "epp/sema/function_table.h"

namespace epp::sema {

// 意味解析済みの関数表と、表中の実行開始関数。
struct AnalyzedProgram {
  FunctionTable function_table;
  FunctionSymbolId entry_point;
  // ASTの関数定義順。IR変換はヘッダを再解析せず、この解決済み番号を使う。
  std::vector<FunctionSymbolId> function_ids;
};

using ProgramAnalysisResult = std::variant<
  AnalyzedProgram,
  Error
>;

// 構文解析済みのソースを、実行プログラムとして意味検査する。
[[nodiscard]] ProgramAnalysisResult AnalyzeProgram(
  const ast::CompilationUnit& compilation_unit,
  const ast::Ast& ast
);

} // namespace epp::sema

#endif // EPP_INCLUDE_EPP_SEMA_PROGRAM_H_
