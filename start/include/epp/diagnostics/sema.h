#ifndef EPP_INCLUDE_EPP_DIAGNOSTICS_SEMA_H_
#define EPP_INCLUDE_EPP_DIAGNOSTICS_SEMA_H_

#include "epp/diagnostics/diagnostic.h"
#include "epp/sema/error.h"

namespace epp::diagnostics {

// 意味解析エラーを共通診断に変換する。
[[nodiscard]] Diagnostic ToDiagnostic(const sema::Error& error);

} // namespace epp::diagnostics

#endif // EPP_INCLUDE_EPP_DIAGNOSTICS_SEMA_H_
