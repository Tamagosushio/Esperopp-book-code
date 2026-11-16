#include "epp/runtime/input.h"

#include <unistd.h>

#include <cerrno>
#include <cstdlib>
#include <new>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "epp/runtime/output.h"
#include "epp/source/utf8.h"
#include "write_all.h"

namespace {

std::vector<std::vector<std::uint32_t>> lines;

[[noreturn]] void Fail(std::string_view message) {
  static_cast<void>(epp::runtime::detail::WriteAll(
    STDERR_FILENO, message.data(), message.size(), ::write
  ));
  std::_Exit(1);
}

std::string ReadLine() {
  throw std::logic_error("not yet implemented: read-line");
}

} // namespace

extern "C" void epp_input_line(
  const std::uint32_t** scalars,
  std::uint64_t* length
) {
  throw std::logic_error("not yet implemented: input-line");
}

extern "C" void epp_input_finish() {
  lines.clear();
}
