#include "epp/diagnostics/diagnostic.h"

#include <string_view>

#include "epp/source/source_position.h"

namespace epp::diagnostics {

namespace {

std::string_view SeverityText(DiagnosticSeverity severity) {
  switch (severity) {
    case DiagnosticSeverity::kError:
      return "error";
    case DiagnosticSeverity::kWarning:
      return "warning";
    case DiagnosticSeverity::kNote:
      return "note";
  }
  return "unknown";
}

} // namespace

std::string FormatDiagnostic(
  const Diagnostic& diagnostic,
  const source::SourceFile& source
) {
  std::string text{source.Path()};
  if (diagnostic.range.has_value()) {
    const auto offset = diagnostic.range->begin_byte_offset;
    const auto position = source::GetSourcePosition(source.Bytes(), offset);
    if (position.has_value()) {
      text += ":" + std::to_string(position->line) + ":" + std::to_string(position->column);
    } else {
      text += ":[byte " + std::to_string(offset) + "]";
    }
  }
  text += ": ";
  text += SeverityText(diagnostic.severity);
  text += ": " + diagnostic.message + "\n";
  return text;
}

} // namespace epp::diagnostics
