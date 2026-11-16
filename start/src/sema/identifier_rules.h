#ifndef EPP_SRC_SEMA_IDENTIFIER_RULES_H_
#define EPP_SRC_SEMA_IDENTIFIER_RULES_H_

#include "epp/morphology/identifier.h"
#include "epp/sema/error.h"

namespace epp::sema::detail {

// 対格語尾-nと複数語尾-jを検査する。正常ならnullopt、不正なら指定範囲のエラーを返す。
inline CheckResult CheckIdentifierEndings(
  const morphology::IdentifierInfo& info, source::SourceRange range
) {
  if (info.has_accusative_ending) {
    return Error{ErrorKind::kAccusativeEndingReserved, range};
  }
  if (info.has_plural_ending) {
    return Error{ErrorKind::kPluralEndingForbidden, range};
  }
  return std::nullopt;
}

} // namespace epp::sema::detail

#endif
