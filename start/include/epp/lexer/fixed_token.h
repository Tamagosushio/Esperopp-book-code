#ifndef EPP_INCLUDE_EPP_LEXER_FIXED_TOKEN_H_
#define EPP_INCLUDE_EPP_LEXER_FIXED_TOKEN_H_

#include <cstddef>
#include <optional>
#include <string_view>

#include "epp/lexer/token.h"

namespace epp::lexer {

// 指定位置から始まる括弧と区切り記号、演算子をトークンに変換する。
// 対応がなければstd::nulloptを返す。
[[nodiscard]] std::optional<Token> LexFixedToken(
  std::string_view source_bytes,
  std::size_t byte_offset
);

} // namespace epp::lexer

#endif // EPP_INCLUDE_EPP_LEXER_FIXED_TOKEN_H_
