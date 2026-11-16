#include "epp/diagnostics/ir_lower.h"

#include <stdexcept>

namespace epp::diagnostics {

Diagnostic ToDiagnostic(const ir::LoweringError& error) {
  const auto make = [&](const char* message) {
    return Diagnostic{DiagnosticSeverity::kError, message, error.range};
  };

  using Kind = ir::LoweringErrorKind;
  switch (error.kind) {
    case Kind::kInvalidStatement:
      return make("invalid statement in function body");
    case Kind::kInvalidExpression:
      return make("invalid expression; use a literal, variable, integer operator, comparison, Boolean operator or direct call");
  }
  throw std::logic_error{"unknown IR lowering error kind"};
}

} // namespace epp::diagnostics
