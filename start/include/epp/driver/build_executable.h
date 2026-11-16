#ifndef EPP_INCLUDE_EPP_DRIVER_BUILD_EXECUTABLE_H_
#define EPP_INCLUDE_EPP_DRIVER_BUILD_EXECUTABLE_H_

#include <filesystem>
#include <optional>
#include <string>

#include "epp/ir/program.h"

namespace epp::driver {

// 実行ファイル生成の失敗理由。外部ツールが失敗した場合は終了コードも保持する。
struct BuildExecutableError {
  std::string message;
  std::optional<int> tool_exit_code = std::nullopt;
};

// IRからmain付き実行ファイルを生成する。
// IRをオブジェクトへ変換し、driverでruntimeとリンクしてoutputへ配置する。
// source、output、driver、runtimeには絶対パスを渡す。sourceと既存outputを保護する。
// 成功はnullopt、失敗は原因を返す。
// timeout_secondsが0ならリンクの時間制限は設けない。
[[nodiscard]] std::optional<BuildExecutableError> BuildExecutableWithGnuDriver(
  const ir::Program& program,
  const std::filesystem::path& source,
  const std::filesystem::path& output,
  const std::filesystem::path& driver,
  const std::filesystem::path& runtime,
  unsigned timeout_seconds = 0
);

} // namespace epp::driver

#endif // EPP_INCLUDE_EPP_DRIVER_BUILD_EXECUTABLE_H_
