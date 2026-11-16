#ifndef EPP_INCLUDE_EPP_FRONTEND_ANALYZE_H_
#define EPP_INCLUDE_EPP_FRONTEND_ANALYZE_H_

#include <variant>

#include "epp/ast/node.h"
#include "epp/lexer/lexer.h"
#include "epp/parser/parser.h"
#include "epp/sema/program.h"
#include "epp/source/source_file.h"

namespace epp::frontend {

// ASTと、そのASTに対するプログラムの意味解析結果を保持する。
struct AnalyzedSource {
  ast::Ast ast;
  ast::NodeId compilation_unit;
  sema::AnalyzedProgram program;
};

// 成功時はASTと意味解析結果、失敗時は最初に検出した段階のエラー。
using SourceAnalysisResult = std::variant<
  AnalyzedSource,
  lexer::LexerError,
  parser::ParserError,
  sema::Error
>;

// 字句、構文、意味解析を順に実行し、最初のエラーを返す。
[[nodiscard]] SourceAnalysisResult AnalyzeSource(
  const source::SourceFile& source
);

} // namespace epp::frontend

#endif // EPP_INCLUDE_EPP_FRONTEND_ANALYZE_H_
