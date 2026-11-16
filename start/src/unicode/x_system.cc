#include "epp/unicode/x_system.h"

#include <cstddef>
#include <optional>

namespace epp::unicode {

namespace {

std::optional<char32_t> GetXSystemReplacement(
  char32_t first,
  char32_t second
) noexcept {
  // 2文字目のxは大文字と小文字の両方を受け付ける。
  if (second != U'x' && second != U'X') {
    return std::nullopt;
  }
  switch (first) {
    case U'c': return U'\u0109'; // ĉ
    case U'g': return U'\u011D'; // ĝ
    case U'h': return U'\u0125'; // ĥ
    case U'j': return U'\u0135'; // ĵ
    case U's': return U'\u015D'; // ŝ
    case U'u': return U'\u016D'; // ŭ
    case U'C': return U'\u0108'; // Ĉ
    case U'G': return U'\u011C'; // Ĝ
    case U'H': return U'\u0124'; // Ĥ
    case U'J': return U'\u0134'; // Ĵ
    case U'S': return U'\u015C'; // Ŝ
    case U'U': return U'\u016C'; // Ŭ
    default: return std::nullopt;
  }
}

} // namespace

} // namespace epp::unicode
