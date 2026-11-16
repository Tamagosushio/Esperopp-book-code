#include "epp/version.h"
#include "epp/version_config.h"

namespace epp {

std::string_view Version() noexcept {
  return EPP_VERSION;
}

std::string VersionText() {
  std::string text = "epp ";
  text += Version();
  return text;
}

}  // namespace epp
