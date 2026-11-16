#ifndef EPP_INCLUDE_EPP_DRIVER_BUILD_ARGUMENTS_H_
#define EPP_INCLUDE_EPP_DRIVER_BUILD_ARGUMENTS_H_

#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <variant>

namespace epp::driver {

// buildで指定した入力パスと、省略可能な-oの出力パス。
struct BuildArguments {
  std::string source_path;
  std::optional<std::string> output_path;
};

// 引数の不足、重複、不明なオプションなどの失敗理由。
struct BuildArgumentsError {
  std::string message;
};

// 成功時は解析したパス、失敗時は引数のエラーを返す。
using BuildArgumentsResult = std::variant<BuildArguments, BuildArgumentsError>;

// executable名とbuildを除いた引数列を解析する。
[[nodiscard]] BuildArgumentsResult ParseBuildArguments(
  std::span<const std::string_view> arguments
);

} // namespace epp::driver

#endif // EPP_INCLUDE_EPP_DRIVER_BUILD_ARGUMENTS_H_
