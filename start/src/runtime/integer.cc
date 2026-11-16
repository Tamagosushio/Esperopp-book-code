#include "epp/runtime/integer.h"

#include <unistd.h>

#include <cstdlib>
#include <string_view>

#include "write_all.h"

extern "C" [[noreturn]] void epp_integer_error() {
  constexpr std::string_view message = "epp runtime error: invalid integer operation\n";
  static_cast<void>(epp::runtime::detail::WriteAll(
    STDERR_FILENO, message.data(), message.size(), ::write
  ));
  std::_Exit(1);
}
