#ifndef EPP_INCLUDE_EPP_UNICODE_X_SYSTEM_H_
#define EPP_INCLUDE_EPP_UNICODE_X_SYSTEM_H_

#include <string>
#include <string_view>

namespace epp::unicode {

// UTF-8からデコード済みの識別子またはキーワード候補をX-system変換する。
[[nodiscard]] std::u32string NormalizeIdentifierXSystem(
  std::u32string_view candidate
);

} // namespace epp::unicode

#endif // EPP_INCLUDE_EPP_UNICODE_X_SYSTEM_H_
