#ifndef EPP_INCLUDE_EPP_SOURCE_UTF8_H_
#define EPP_INCLUDE_EPP_SOURCE_UTF8_H_

#include <cstddef>
#include <optional>
#include <string_view>

namespace epp::source {

// 先頭にUTF-8 BOMがある場合、3バイトを読み飛ばす。
[[nodiscard]] std::string_view StripUtf8Bom(
  std::string_view bytes
) noexcept;

// UTF-8から変換したUnicode文字を表す。
struct DecodedUtf8Scalar {
  // Unicodeスカラー値。
  char32_t value;
  // UTF-8で使用したバイト数。
  std::size_t byte_count;
};

// bytesのoffsetから始まるUTF-8を1文字だけ変換する。
// 変換できない場合や範囲外のときにはstd::nulloptを返す。
[[nodiscard]] std::optional<DecodedUtf8Scalar> DecodeUtf8Scalar(
  std::string_view bytes,
  std::size_t offset
) noexcept;

// bytesが正しいUTF-8なら、std::nulloptを返す。
// 不正なUTF-8なら、不正部分が始まる位置を返す。
[[nodiscard]] std::optional<std::size_t> FindInvalidUtf8(
  std::string_view bytes
) noexcept;

} // namespace epp::source

#endif // EPP_INCLUDE_EPP_SOURCE_UTF8_H_
