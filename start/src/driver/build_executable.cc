#include "epp/driver/build_executable.h"

#include <llvm/ADT/SmallString.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Program.h>

#include <algorithm>
#include <fstream>
#include <span>
#include <stdexcept>
#include <system_error>
#include <utility>

#include "epp/codegen/object.h"

namespace epp::driver {

namespace {

std::string Utf8Path(const std::filesystem::path& path) {
  const auto utf8 = path.u8string();
  return {utf8.begin(), utf8.end()};
}

bool IsValidPath(const std::filesystem::path& path) {
  return path.is_absolute()
    && path.native().find(std::filesystem::path::value_type{}) == std::filesystem::path::string_type::npos;
}

std::optional<BuildExecutableError> ValidateBuildOutput(
  const std::filesystem::path& source,
  const std::filesystem::path& output
) {
  namespace fs = std::filesystem;
  if (!IsValidPath(source) || !IsValidPath(output) || output.filename().empty()) {
    return BuildExecutableError{"source and output must be absolute file paths without NUL"};
  }

  std::error_code error;
  const bool regular_source = fs::is_regular_file(source, error);
  if (error || !regular_source) {
    return BuildExecutableError{"source must be an accessible regular file"};
  }

  const bool directory_parent = fs::is_directory(output.parent_path(), error);
  if (error || !directory_parent) {
    return BuildExecutableError{"output parent must be an accessible directory"};
  }

  const auto status = fs::symlink_status(output, error);
  if (status.type() == fs::file_type::not_found
    && (!error || error == std::errc::no_such_file_or_directory)) {
    return std::nullopt;
  }
  if (error) {
    return BuildExecutableError{"cannot inspect output path"};
  }
  if (!fs::is_regular_file(status)) {
    return BuildExecutableError{"existing output must be a regular file, not a symlink"};
  }

  const bool same = fs::equivalent(source, output, error);
  if (error) {
    return BuildExecutableError{"cannot compare source and output"};
  }
  if (same) {
    return BuildExecutableError{"output must not refer to the source file"};
  }
  return std::nullopt;
}

std::optional<BuildExecutableError> PublishBuildOutput(
  const std::filesystem::path& source,
  const std::filesystem::path& staged,
  const std::filesystem::path& output
) {
  if (const auto error = ValidateBuildOutput(source, output)) {
    return error;
  }
  if (const auto error = ValidateBuildOutput(source, staged)) {
    return BuildExecutableError{"invalid staged file: " + error->message};
  }
  if (const auto error = ValidateBuildOutput(staged, output)) {
    return BuildExecutableError{"cannot publish staged file: " + error->message};
  }

  std::error_code error;
  std::filesystem::rename(staged, output, error);
  if (error) {
    return BuildExecutableError{"cannot publish output: " + error.message()};
  }
  return std::nullopt;
}

class TemporaryDirectory {
  public:
  std::filesystem::path path;
  TemporaryDirectory() = default;
  TemporaryDirectory(const TemporaryDirectory&) = delete;
  TemporaryDirectory& operator=(const TemporaryDirectory&) = delete;
  ~TemporaryDirectory() noexcept {
    try {
      if (!path.empty()) {
        std::error_code error;
        std::filesystem::remove_all(path, error);
      }
    } catch (...) {
    }
  }

  std::error_code Create(const std::filesystem::path& parent) {
    for (int attempt = 0; attempt < 128; ++attempt) {
      llvm::SmallString<64> name;
      llvm::sys::fs::createUniquePath(".epp-build-%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%", name, false);
      auto candidate = parent / std::filesystem::u8path(name.begin(), name.end()).filename();
      const auto error = llvm::sys::fs::create_directory(
        Utf8Path(candidate), false, llvm::sys::fs::owner_all
      );
      if (!error) {
        path = std::move(candidate);
        return {};
      }
      if (error != std::errc::file_exists) {
        return error;
      }
    }
    return std::make_error_code(std::errc::file_exists);
  }
};

std::optional<BuildExecutableError> Link(
  const std::filesystem::path& driver,
  const std::filesystem::path& object,
  const std::filesystem::path& staged,
  const std::filesystem::path& runtime,
  unsigned timeout_seconds
) {
  throw std::logic_error("not yet implemented: link");
}

std::optional<BuildExecutableError> WriteOutputFile(
  const std::filesystem::path& path,
  std::span<const char> bytes
) {
  std::ofstream output{path, std::ios::binary | std::ios::trunc};
  if (!output.is_open()) {
    return BuildExecutableError{"cannot open temporary object file"};
  }

  constexpr std::size_t kWriteChunkSize = 4096;
  while (!bytes.empty()) {
    const auto count = std::min(bytes.size(), kWriteChunkSize);
    output.write(bytes.data(), static_cast<std::streamsize>(count));
    if (!output) {
      return BuildExecutableError{"cannot write temporary object file"};
    }
    bytes = bytes.subspan(count);
  }
  output.close();
  if (!output) {
    return BuildExecutableError{"cannot write temporary object file"};
  }
  return std::nullopt;
}

} // namespace

std::optional<BuildExecutableError> BuildExecutableWithGnuDriver(
  const ir::Program& program,
  const std::filesystem::path& source,
  const std::filesystem::path& output,
  const std::filesystem::path& driver,
  const std::filesystem::path& runtime,
  unsigned timeout_seconds
) {
  throw std::logic_error("not yet implemented: build-executable");
}

} // namespace epp::driver
