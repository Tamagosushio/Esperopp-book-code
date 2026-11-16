#include "epp/sema/integer_literal.h"

#include <limits>
#include <stdexcept>

namespace epp::sema {

std::optional<std::int64_t> FormIntegerLiteralValue(
  std::string_view digits,
  bool negate
) {
  std::uint64_t limit = std::numeric_limits<std::int64_t>::max();
  if (negate) {
    ++limit;
  }

  if (digits.empty()) {
    throw std::logic_error{"empty integer literal digits"};
  }
  for (const char digit : digits) {
    if (digit < '0' || digit > '9') {
      throw std::logic_error{"invalid integer literal digits"};
    }
  }

  std::uint64_t magnitude = 0;
  for (const char digit : digits) {
    const auto value = static_cast<std::uint64_t>(digit - '0');
    if (magnitude > (limit - value) / 10) {
      return std::nullopt;
    }
    magnitude = magnitude * 10 + value;
  }

  if (!negate) {
    return static_cast<std::int64_t>(magnitude);
  }
  const auto min_magnitude = static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max()) + 1;
  if (magnitude == min_magnitude) {
    return std::numeric_limits<std::int64_t>::min();
  }
  return -static_cast<std::int64_t>(magnitude);
}

} // namespace epp::sema
