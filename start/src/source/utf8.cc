#include "epp/source/utf8.h"

namespace epp::source {

namespace {

using Utf8Byte = unsigned char;

Utf8Byte ByteAt(std::string_view bytes, std::size_t index) noexcept {
  return static_cast<Utf8Byte>(bytes[index]);
}

bool IsInRange(Utf8Byte byte, Utf8Byte lower, Utf8Byte upper) noexcept {
  return lower <= byte && byte <= upper;
}

bool IsContinuationByte(Utf8Byte byte) noexcept {
  return IsInRange(byte, 0x80, 0xBF);
}

bool HasValidSecondByteForThreeByteSequence(Utf8Byte first, Utf8Byte second) noexcept {
  if (first == 0xE0) {
    return IsInRange(second, 0xA0, 0xBF);
  }
  if (first == 0xED) {
    return IsInRange(second, 0x80, 0x9F);
  }
  return IsContinuationByte(second);
}

bool HasValidSecondByteForFourByteSequence(Utf8Byte first, Utf8Byte second) noexcept {
  if (first == 0xF0) {
    return IsInRange(second, 0x90, 0xBF);
  }
  if (first == 0xF4) {
    return IsInRange(second, 0x80, 0x8F);
  }
  return IsContinuationByte(second);
}

} // namespace

std::string_view StripUtf8Bom(std::string_view bytes) noexcept {
  constexpr std::string_view kUtf8Bom{"\xEF\xBB\xBF", 3};
  if (bytes.starts_with(kUtf8Bom)) {
    bytes.remove_prefix(kUtf8Bom.size());
  }
  return bytes;
}

std::optional<DecodedUtf8Scalar> DecodeUtf8Scalar(
  std::string_view bytes,
  std::size_t offset
) noexcept {
  if (offset >= bytes.size()) {
    return std::nullopt;
  }
  const Utf8Byte first = ByteAt(bytes, offset);

  // ASCII
  if (first <= 0x7F) {
    return DecodedUtf8Scalar{static_cast<char32_t>(first), 1};
  }

  // 2byte文字
  if (IsInRange(first, 0xC2, 0xDF)) {
    if (bytes.size() - offset < 2) {
      return std::nullopt;
    }
    const Utf8Byte second = ByteAt(bytes, offset + 1);
    if (!IsContinuationByte(second)) {
      return std::nullopt;
    }
    const char32_t value
      = (static_cast<char32_t>(first & 0x1F) << 6)
      | static_cast<char32_t>(second & 0x3F);
    return DecodedUtf8Scalar{value, 2};
  }

  // 3byte文字
  if (IsInRange(first, 0xE0, 0xEF)) {
    if (bytes.size() - offset < 3) {
      return std::nullopt;
    }
    const Utf8Byte second = ByteAt(bytes, offset + 1);
    const Utf8Byte third = ByteAt(bytes, offset + 2);
    if (!HasValidSecondByteForThreeByteSequence(first, second)
      || !IsContinuationByte(third)) {
      return std::nullopt;
    }
    const char32_t value
      = (static_cast<char32_t>(first & 0x0F) << 12)
      | (static_cast<char32_t>(second & 0x3F) << 6)
      | static_cast<char32_t>(third & 0x3F);
    return DecodedUtf8Scalar{value, 3};
  }

  // 4byte文字
  if (IsInRange(first, 0xF0, 0xF4)) {
    if (bytes.size() - offset < 4) {
      return std::nullopt;
    }
    const Utf8Byte second = ByteAt(bytes, offset + 1);
    const Utf8Byte third = ByteAt(bytes, offset + 2);
    const Utf8Byte fourth = ByteAt(bytes, offset + 3);
    if (!HasValidSecondByteForFourByteSequence(first, second)
      || !IsContinuationByte(third)
      || !IsContinuationByte(fourth)) {
      return std::nullopt;
    }
    const char32_t value
      = (static_cast<char32_t>(first & 0x07) << 18)
      | (static_cast<char32_t>(second & 0x3F) << 12)
      | (static_cast<char32_t>(third & 0x3F) << 6)
      | static_cast<char32_t>(fourth & 0x3F);
    return DecodedUtf8Scalar{value, 4};
  }

  return std::nullopt;
}

std::optional<std::size_t> FindInvalidUtf8(std::string_view bytes) noexcept {
  std::size_t offset = 0;

  while (offset < bytes.size()) {
    const auto decoded = DecodeUtf8Scalar(bytes, offset);
    if (!decoded.has_value()) {
      return offset;
    }
    offset += decoded->byte_count;
  }

  return std::nullopt;
}

} // namespace epp::source
