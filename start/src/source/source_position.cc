#include "epp/source/source_position.h"

#include "epp/source/utf8.h"

namespace epp::source {

SourcePosition GetSourceStartPosition(std::string_view bytes) noexcept {
  const std::string_view source_without_bom = StripUtf8Bom(bytes);
  const std::size_t byte_offset = bytes.size() - source_without_bom.size();
  return SourcePosition{byte_offset, 1, 1};
}

std::optional<SourcePosition> AdvanceSourcePosition(
  std::string_view bytes,
  SourcePosition position
) noexcept {
  const auto decoded = DecodeUtf8Scalar(bytes, position.byte_offset);
  if (!decoded.has_value()) {
    return std::nullopt;
  }
  SourcePosition next = position;
  next.byte_offset += decoded->byte_count;
  if (decoded->value == U'\r') {
    if (next.byte_offset < bytes.size()
      && bytes[next.byte_offset] == '\n') {
      ++next.byte_offset;
    }
    ++next.line;
    next.column = 1;
    return next;
  }
  if (decoded->value == U'\n') {
    ++next.line;
    next.column = 1;
    return next;
  }
  ++next.column;
  return next;
}

std::optional<SourcePosition> GetSourcePosition(
  std::string_view bytes,
  std::size_t byte_offset
) noexcept {
  auto position = GetSourceStartPosition(bytes);
  if (byte_offset < position.byte_offset || byte_offset > bytes.size()) {
    return std::nullopt;
  }
  while (position.byte_offset < byte_offset) {
    const auto next = AdvanceSourcePosition(bytes, position);
    if (!next.has_value() || next->byte_offset > byte_offset) {
      return std::nullopt;
    }
    position = *next;
  }
  return position;
}

} // namespace epp::source
