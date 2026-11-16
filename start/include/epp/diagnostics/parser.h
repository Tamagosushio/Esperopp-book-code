#ifndef EPP_INCLUDE_EPP_DIAGNOSTICS_PARSER_H_
#define EPP_INCLUDE_EPP_DIAGNOSTICS_PARSER_H_

#include "epp/diagnostics/diagnostic.h"
#include "epp/parser/parser.h"

namespace epp::diagnostics {

// 構文解析エラーを共通診断に変換する。
[[nodiscard]] Diagnostic ToDiagnostic(const parser::ParserError& error);

} // namespace epp::diagnostics

#endif // EPP_INCLUDE_EPP_DIAGNOSTICS_PARSER_H_
