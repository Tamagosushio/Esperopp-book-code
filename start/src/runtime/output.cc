#include "epp/runtime/output.h"

#include <unistd.h>

#include <stdexcept>
#include <algorithm>
#include <charconv>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <string_view>

#include "write_all.h"

namespace {

char buffer[4096];
std::size_t used = 0;

[[noreturn]] void Fail() {
  constexpr std::string_view message = "epp runtime error: output failure\n";
  static_cast<void>(epp::runtime::detail::WriteAll(
    STDERR_FILENO, message.data(), message.size(), ::write
  ));
  // pending outputの再flushや利用者の終了処理は行わない。
  std::_Exit(1);
}

void Flush() {
  if (!epp::runtime::detail::WriteAll(STDOUT_FILENO, buffer, used, ::write)) {
    Fail();
  }
  used = 0;
}

void Append(std::string_view bytes) {
  throw std::logic_error("not yet implemented: append-output");
}

} // namespace

extern "C" void epp_output_init() {
  struct sigaction action{};
  action.sa_handler = SIG_IGN;
  if (sigemptyset(&action.sa_mask) != 0 || sigaction(SIGPIPE, &action, nullptr) != 0) {
    Fail();
  }
}

extern "C" void epp_output_integer(std::int64_t value) {
  throw std::logic_error("not yet implemented: output-integer");
}

extern "C" void epp_output_boolean(std::uint32_t value) {
  Append(value == 0 ? "malvera" : "vera");
}

extern "C" void epp_output_character(std::uint32_t scalar) {
  throw std::logic_error("not yet implemented: output-character");
}

extern "C" void epp_output_string(
  const std::uint32_t* scalars,
  std::uint64_t length
) {
  for (std::uint64_t i = 0; i < length; ++i) {
    epp_output_character(scalars[i]);
  }
}

extern "C" void epp_output_flush() {
  Flush();
}

extern "C" void epp_output_finish() {
  epp_output_flush();
}
