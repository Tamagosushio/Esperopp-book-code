#ifndef EPP_INCLUDE_EPP_DIAGNOSTICS_DIAGNOSTIC_H_
#define EPP_INCLUDE_EPP_DIAGNOSTICS_DIAGNOSTIC_H_

#include <optional>
#include <string>

#include "epp/source/source_file.h"
#include "epp/source/source_range.h"

namespace epp::diagnostics {

// 診断の種類。処理系のエラー、警告、補足情報を区別する。
enum class DiagnosticSeverity {
  kError,
  kWarning,
  kNote,
};

// 各解析段階で共通に表示する診断。rangeを持たない診断はファイル全体に対するもの。
struct Diagnostic {
  DiagnosticSeverity severity;
  std::string message;
  std::optional<source::SourceRange> range;
};

// 診断の先頭行を返す。
[[nodiscard]] std::string FormatDiagnostic(
  const Diagnostic& diagnostic,
  const source::SourceFile& source
);

} // namespace epp::diagnostics

#endif // EPP_INCLUDE_EPP_DIAGNOSTICS_DIAGNOSTIC_H_
