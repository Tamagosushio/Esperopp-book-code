#ifndef EPP_SRC_RUNTIME_WRITE_ALL_H_
#define EPP_SRC_RUNTIME_WRITE_ALL_H_

#include <cerrno>
#include <cstddef>

namespace epp::runtime::detail {

// writeの部分成功とEINTRを処理する。0やその他の失敗で再試行し続けない。
template <typename Write>
bool WriteAll(int descriptor, const char* bytes, std::size_t length, Write write) {
  while (length != 0) {
    const auto count = write(descriptor, bytes, length);
    if (count < 0 && errno == EINTR) {
      continue;
    }
    if (count <= 0 || static_cast<std::size_t>(count) > length) {
      return false;
    }
    bytes += count;
    length -= static_cast<std::size_t>(count);
  }
  return true;
}

} // namespace epp::runtime::detail

#endif // EPP_SRC_RUNTIME_WRITE_ALL_H_
