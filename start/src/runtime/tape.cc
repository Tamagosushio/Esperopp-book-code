#include "epp/runtime/tape.h"

#include <unistd.h>

#include <stdexcept>
#include <cstdlib>
#include <new>
#include <string_view>

#include "write_all.h"

namespace {

struct Cell {
  Cell* left = nullptr;
  Cell* right = nullptr;
  std::int64_t value = 0;
};

Cell origin;
Cell* head = &origin;
[[noreturn]] void Fail() {
  constexpr std::string_view message = "epp runtime error: tape allocation failure\n";
  static_cast<void>(epp::runtime::detail::WriteAll(
    STDERR_FILENO, message.data(), message.size(), ::write
  ));
  std::_Exit(1);
}

} // namespace

extern "C" std::int64_t epp_tape_step(std::int64_t value, std::uint32_t right) {
  throw std::logic_error("not yet implemented: step");
}

extern "C" void epp_tape_finish() {
  throw std::logic_error("not yet implemented: finish");
}
