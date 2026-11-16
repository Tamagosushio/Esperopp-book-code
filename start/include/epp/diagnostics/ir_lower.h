#ifndef EPP_INCLUDE_EPP_DIAGNOSTICS_IR_LOWER_H_
#define EPP_INCLUDE_EPP_DIAGNOSTICS_IR_LOWER_H_

#include "epp/diagnostics/diagnostic.h"
#include "epp/ir/lower.h"

namespace epp::diagnostics {

// IR変換で検出したエラーを共通診断へ変換する。
[[nodiscard]] Diagnostic ToDiagnostic(const ir::LoweringError& error);

} // namespace epp::diagnostics

#endif // EPP_INCLUDE_EPP_DIAGNOSTICS_IR_LOWER_H_
