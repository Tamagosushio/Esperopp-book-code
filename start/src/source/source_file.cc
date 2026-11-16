#include "epp/source/source_file.h"

#include <array>
#include <fstream>
#include <string>
#include <utility>

#include "epp/source/utf8.h"

namespace epp::source {

SourceFile::SourceFile(std::string path, std::string bytes)
  : path_(std::move(path)), bytes_(std::move(bytes)) {}

std::string_view SourceFile::Path() const noexcept {
  return path_;
}

std::string_view SourceFile::Bytes() const noexcept {
  return bytes_;
}

std::optional<std::size_t>
SourceFile::FindInvalidUtf8Offset() const noexcept {
  return FindInvalidUtf8(bytes_);
}

SourcePosition SourceFile::StartPosition() const noexcept {
  return GetSourceStartPosition(bytes_);
}

SourceRange SourceFile::ContentRange() const noexcept {
  const SourcePosition start = StartPosition();
  return SourceRange{start.byte_offset, bytes_.size()};
}

SourceFileReadResult ReadSourceFile(const std::filesystem::path& path) {
  // バイナリで読み込み、OS依存の改行コードの変換を防ぐ。
  std::ifstream input{path, std::ios::binary};
  if (!input.is_open()) {
    return SourceFileReadResult{SourceFileReadError::kCannotOpen};
  }
  std::string bytes;
  // ファイルを一定量ずつ読み込む。
  constexpr std::size_t kReadBufferSize = 4096;
  std::array<char, kReadBufferSize> buffer{};
  while (input) {
    input.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
    const std::streamsize read_count = input.gcount();
    if (read_count > 0) {
      bytes.append(buffer.data(), static_cast<std::size_t>(read_count));
    }
  }
  // ファイル末尾以外で読み込みが終了したら失敗とする。
  if (input.bad() || !input.eof()) {
    return SourceFileReadResult{SourceFileReadError::kCannotRead};
  }
  return SourceFileReadResult{SourceFile{path.string(), std::move(bytes)}};
}

} // namespace epp::source
