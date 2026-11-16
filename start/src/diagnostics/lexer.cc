#include "epp/diagnostics/lexer.h"

#include <stdexcept>

namespace epp::diagnostics {

Diagnostic ToDiagnostic(const lexer::LexerError& error) {
  const auto make = [&](const char* message) {
    return Diagnostic{
      DiagnosticSeverity::kError, message, error.range
    };
  };

  using lexer::LexerErrorKind;
  switch (error.kind) {
    case LexerErrorKind::kUnterminatedBlockComment:
      return make("unterminated block comment");
    case LexerErrorKind::kUnterminatedCharacterLiteral:
      return make("unterminated character literal");
    case LexerErrorKind::kUnterminatedStringLiteral:
      return make("unterminated string literal");
    case LexerErrorKind::kInvalidUtf8:
      return make("ill-formed UTF-8");
    case LexerErrorKind::kInvalidEscapeSequence:
      return make("invalid escape sequence");
    case LexerErrorKind::kInvalidCharacterLiteralLength:
      return make("character literal must contain exactly one Unicode scalar");
    case LexerErrorKind::kInvalidCharacter:
      return make("invalid character");
    case LexerErrorKind::kInvalidIntegerLiteral:
      return make("invalid integer literal");
  }
  throw std::logic_error{"unknown lexer error kind"};
}

} // namespace epp::diagnostics
