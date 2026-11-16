#ifndef EPP_INCLUDE_EPP_SEMA_INTEGER_LITERAL_H_
#define EPP_INCLUDE_EPP_SEMA_INTEGER_LITERAL_H_

#include <cstdint>
#include <optional>
#include <string_view>

namespace epp::sema {

// 数字列から符号付き64ビット整数の値を形成する。
// negateは、リテラルが単項マイナスの直接の被演算子の場合だけtrueにする。
[[nodiscard]] std::optional<std::int64_t> FormIntegerLiteralValue(
  std::string_view digits,
  bool negate
);

} // namespace epp::sema

#endif // EPP_INCLUDE_EPP_SEMA_INTEGER_LITERAL_H_
