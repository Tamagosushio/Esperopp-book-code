#ifndef EPP_INCLUDE_EPP_IR_LOWER_H_
#define EPP_INCLUDE_EPP_IR_LOWER_H_

#include <variant>

#include "epp/frontend/analyze.h"
#include "epp/ir/program.h"

namespace epp::ir {

// IRへの変換時に検出した不正な型・文・式を表す。
enum class LoweringErrorKind {
  kInvalidStatement,
  kInvalidExpression,
};

// IRへ変換できないASTの種類と、元ソース上の検出範囲。
struct LoweringError {
  LoweringErrorKind kind;
  source::SourceRange range;
};

// 成功時はプログラム全体のIR、失敗時は変換エラー。
using LoweringResult = std::variant<Program, LoweringError>;

// AnalyzeSourceが成功して返した結果を渡す。
// ASTと関数表を変更せず、変数スロットと制御の基本ブロックを持つIRを作る。
// 意味解析結果の内部不整合ではlogic_errorを送出する。
[[nodiscard]] LoweringResult LowerToIr(const frontend::AnalyzedSource& source);

} // namespace epp::ir

#endif // EPP_INCLUDE_EPP_IR_LOWER_H_
