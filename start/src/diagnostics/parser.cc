#include "epp/diagnostics/parser.h"

#include <stdexcept>

namespace epp::diagnostics {

Diagnostic ToDiagnostic(const parser::ParserError& error) {
  const auto make = [&](const char* message) {
    return Diagnostic{
      DiagnosticSeverity::kError, message, error.range
    };
  };

  using parser::ParserErrorKind;
  switch (error.kind) {
    case ParserErrorKind::kExpectedExpression:
      return make("expected an expression");
    case ParserErrorKind::kExpectedLeftParenthesis:
      return make("expected '('");
    case ParserErrorKind::kExpectedRightParenthesis:
      return make("expected ')'");
    case ParserErrorKind::kExpectedCommaOrRightParenthesis:
      return make("expected ',' or ')'");
    case ParserErrorKind::kExpectedAssignmentTarget:
      return make("expected an assignment target");
    case ParserErrorKind::kExpectedAssignmentOperator:
      return make("expected an assignment operator");
    case ParserErrorKind::kExpectedSemicolon:
      return make("expected ';'");
    case ParserErrorKind::kExpectedCallExpression:
      return make("expected a function call");
    case ParserErrorKind::kExpectedReturnKeyword:
      return make("expected 'liveru'");
    case ParserErrorKind::kExpectedLeftBrace:
      return make("expected '{'");
    case ParserErrorKind::kExpectedRightBrace:
      return make("expected '}'");
    case ParserErrorKind::kExpectedStatement:
      return make("expected a statement");
    case ParserErrorKind::kExpectedDeclarationComponent:
      return make("expected a declaration component");
    case ParserErrorKind::kExpectedEqualOrSemicolon:
      return make("expected '=' or ';'");
    case ParserErrorKind::kExpectedIfKeyword:
      return make("expected 'se'");
    case ParserErrorKind::kExpectedWhileKeyword:
      return make("expected 'dum'");
    case ParserErrorKind::kExpectedBreakKeyword:
      return make("expected 'rompu'");
    case ParserErrorKind::kExpectedContinueKeyword:
      return make("expected 'daŭrigu'");
    case ParserErrorKind::kExpectedFunctionHeadComponent:
      return make("expected a function header component");
  }
  throw std::logic_error{"unknown parser error kind"};
}

} // namespace epp::diagnostics
