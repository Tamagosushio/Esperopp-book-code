#include "epp/driver/build_arguments.h"

#include <cstddef>

namespace epp::driver {

BuildArgumentsResult ParseBuildArguments(
  std::span<const std::string_view> arguments
) {
  for (const auto argument : arguments) {
    if (argument.empty() || argument.find('\0') != std::string_view::npos) {
      return BuildArgumentsError{"arguments must not be empty or contain NUL"};
    }
  }

  BuildArguments result;
  bool parse_options = true;
  for (std::size_t i = 0; i < arguments.size(); ++i) {
    const auto argument = arguments[i];
    if (parse_options && argument == "--") {
      parse_options = false;
      continue;
    }
    if (parse_options && argument == "-o") {
      if (result.output_path.has_value()) {
        return BuildArgumentsError{"-o must not be repeated"};
      }
      if (i+1 == arguments.size() || arguments[i+1].starts_with('-')) {
        return BuildArgumentsError{"-o requires an output path; use ./ for a leading '-'"};
      }
      result.output_path = std::string{arguments[++i]};
      continue;
    }
    if (parse_options && argument.starts_with('-')) {
      return BuildArgumentsError{"unknown build option: " + std::string{argument}};
    }
    if (!result.source_path.empty()) {
      return BuildArgumentsError{"exactly one source path is required"};
    }
    result.source_path = argument;
  }
  if (result.source_path.empty()) {
    return BuildArgumentsError{"source path is required"};
  }
  return result;
}

} // namespace epp::driver
