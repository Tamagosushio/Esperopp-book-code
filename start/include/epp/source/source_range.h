#ifndef EPP_INCLUDE_EPP_SOURCE_SOURCE_RANGE_H_
#define EPP_INCLUDE_EPP_SOURCE_SOURCE_RANGE_H_

#include <cstddef>

namespace epp::source {

// 元ソース上のバイト範囲を表す。
// [begin_byte_offset, end_byte_offset)
struct SourceRange {
  std::size_t begin_byte_offset = 0;
  std::size_t end_byte_offset = 0;
  // 開始位置と終了位置が等しいか比較する。
  [[nodiscard]] bool operator==(const SourceRange& other) const noexcept = default;
};

} // namespace epp::source

#endif // EPP_INCLUDE_EPP_SOURCE_SOURCE_RANGE_H_
