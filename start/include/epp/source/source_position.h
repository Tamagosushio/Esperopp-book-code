#ifndef EPP_INCLUDE_EPP_SOURCE_SOURCE_POSITION_H_
#define EPP_INCLUDE_EPP_SOURCE_SOURCE_POSITION_H_

#include <cstddef>
#include <optional>
#include <string_view>

namespace epp::source {

// ソース上の位置を表す。
// 列はUnicodeスカラー値の数で数える。
struct SourcePosition {
  std::size_t byte_offset = 0;
  std::size_t line = 1;
  std::size_t column = 1;
};

// ソースを読み始める位置を返す。
// 先頭のUTF-8 BOMは読み飛ばす。
[[nodiscard]] SourcePosition GetSourceStartPosition(
  std::string_view bytes
) noexcept;

// 現在位置からUnicodeを1文字進めた位置を返す。
// CRLFは一つの改行として扱う。
// 終端や不正なUTF-8ではnulloptを返す。
[[nodiscard]] std::optional<SourcePosition> AdvanceSourcePosition(
  std::string_view bytes,
  SourcePosition position
) noexcept;

// バイト位置に対応する行数と列数を返す。
// 範囲外の位置や、BOM・UTF-8・CRLFの途中の位置ではnulloptを返す。
[[nodiscard]] std::optional<SourcePosition> GetSourcePosition(
  std::string_view bytes,
  std::size_t byte_offset
) noexcept;

} // namespace epp::source

#endif // EPP_INCLUDE_EPP_SOURCE_SOURCE_POSITION_H_
