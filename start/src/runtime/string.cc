#include "epp/runtime/string.h"

#include <unistd.h>

#include <cstdlib>
#include <limits>
#include <stdexcept>
#include <string_view>

#include "write_all.h"

namespace {

[[noreturn]] void Fail(std::string_view message) {
  static_cast<void>(epp::runtime::detail::WriteAll(
    STDERR_FILENO, message.data(), message.size(), ::write
  ));
  std::_Exit(1);
}

} // namespace

extern "C" std::int64_t epp_string_length(std::uint64_t length) {
  throw std::logic_error("not yet implemented: string-length");
}

extern "C" std::uint32_t epp_string_character(
  const std::uint32_t* characters,
  std::uint64_t length,
  std::int64_t index
) {
  throw std::logic_error("not yet implemented: string-character");
}
