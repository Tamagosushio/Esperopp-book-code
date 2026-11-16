#include "epp/diagnostics/source_analysis.h"

#include <type_traits>
#include <variant>

#include "epp/diagnostics/lexer.h"
#include "epp/diagnostics/parser.h"
#include "epp/diagnostics/sema.h"

namespace epp::diagnostics {

std::optional<Diagnostic> ToDiagnostic(const frontend::SourceAnalysisResult& result) {
  return std::visit([](const auto& value) -> std::optional<Diagnostic> {
    using Value = std::decay_t<decltype(value)>;
    if constexpr (std::is_same_v<Value, frontend::AnalyzedSource>) {
      return std::nullopt;
    } else {
      static_assert(
        std::is_same_v<decltype(ToDiagnostic(value)), Diagnostic>,
        "each source error must have a Diagnostic conversion"
      );
      return ToDiagnostic(value);
    }},
    result
  );
}

} // namespace epp::diagnostics
