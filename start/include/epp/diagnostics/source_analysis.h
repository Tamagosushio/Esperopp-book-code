#ifndef EPP_INCLUDE_EPP_DIAGNOSTICS_SOURCE_ANALYSIS_H_
#define EPP_INCLUDE_EPP_DIAGNOSTICS_SOURCE_ANALYSIS_H_

#include <optional>

#include "epp/diagnostics/diagnostic.h"
#include "epp/frontend/analyze.h"

namespace epp::diagnostics {

// 成功なら診断なし、失敗なら既存のエラー変換による診断を返す。
[[nodiscard]] std::optional<Diagnostic> ToDiagnostic(
  const frontend::SourceAnalysisResult& result
);

} // namespace epp::diagnostics

#endif // EPP_INCLUDE_EPP_DIAGNOSTICS_SOURCE_ANALYSIS_H_
