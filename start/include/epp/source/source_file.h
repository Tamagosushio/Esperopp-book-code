#ifndef EPP_INCLUDE_EPP_SOURCE_SOURCE_FILE_H_
#define EPP_INCLUDE_EPP_SOURCE_SOURCE_FILE_H_

#include <cstddef>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <variant>

#include "epp/source/source_position.h"
#include "epp/source/source_range.h"

namespace epp::source {

// ソースファイルのパスとバイト列を保持する。
class SourceFile {
 public:
  SourceFile(std::string path, std::string bytes);
  // ソースファイルのパスを返す。
  [[nodiscard]] std::string_view Path() const noexcept;
  // BOMを含む元のバイト列を返す。
  [[nodiscard]] std::string_view Bytes() const noexcept;
  // UTF-8が正しければstd::nulloptを返す。
  [[nodiscard]] std::optional<std::size_t> FindInvalidUtf8Offset() const noexcept;
  // BOMを読み飛ばした本文の開始位置を返す。
  [[nodiscard]] SourcePosition StartPosition() const noexcept;
  // BOMを除いた本文全体のバイト範囲を返す。
  [[nodiscard]] SourceRange ContentRange() const noexcept;

 private:
  std::string path_;
  std::string bytes_;
};

// ソースファイルを読み込めない原因を表す。
enum class SourceFileReadError {
  kCannotOpen,
  kCannotRead,
};

// 読み込みに成功したらSourceFile、失敗したら原因を保持する。
using SourceFileReadResult = std::variant<SourceFile, SourceFileReadError>;

// ソースファイルをバイト列として読み込む。
[[nodiscard]] SourceFileReadResult ReadSourceFile(const std::filesystem::path& path);

} // namespace epp::source

#endif // EPP_INCLUDE_EPP_SOURCE_SOURCE_FILE_H_
