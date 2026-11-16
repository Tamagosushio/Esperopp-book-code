#ifndef EPP_INCLUDE_EPP_RUNTIME_STRING_H_
#define EPP_INCLUDE_EPP_RUNTIME_STRING_H_

#include <cstdint>

// 文字数を返す。entjeraの範囲を超える場合は診断して終了する。
extern "C" std::int64_t epp_string_length(std::uint64_t length);

// 0から数えた位置のUnicodeスカラー値を返す。範囲外は診断して終了する。
extern "C" std::uint32_t epp_string_character(
  const std::uint32_t* characters,
  std::uint64_t length,
  std::int64_t index
);

#endif // EPP_INCLUDE_EPP_RUNTIME_STRING_H_
