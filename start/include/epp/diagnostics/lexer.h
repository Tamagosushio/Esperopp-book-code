#ifndef EPP_INCLUDE_EPP_DIAGNOSTICS_LEXER_H_
#define EPP_INCLUDE_EPP_DIAGNOSTICS_LEXER_H_

#include "epp/diagnostics/diagnostic.h"
#include "epp/lexer/lexer.h"

namespace epp::diagnostics {

// 字句解析エラーを共通診断型に変換する。
[[nodiscard]] Diagnostic ToDiagnostic(const lexer::LexerError& error);

} // namespace epp::diagnostics

#endif // EPP_INCLUDE_EPP_DIAGNOSTICS_LEXER_H_
