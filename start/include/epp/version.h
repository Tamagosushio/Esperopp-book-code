#ifndef EPP_INCLUDE_EPP_VERSION_H_
#define EPP_INCLUDE_EPP_VERSION_H_

#include <string>
#include <string_view>

namespace epp {

[[nodiscard]] std::string_view Version() noexcept;
[[nodiscard]] std::string VersionText();

} // namespace epp

#endif // EPP_INCLUDE_EPP_VERSION_H_
