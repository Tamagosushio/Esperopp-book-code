#ifndef EPP_INCLUDE_EPP_LEXER_LEXER_H_
#define EPP_INCLUDE_EPP_LEXER_LEXER_H_

#include <cstddef>
#include <optional>
#include <string_view>
#include <variant>
#include <vector>

#include "epp/lexer/token.h"
#include "epp/source/source_range.h"

namespace epp::lexer {

// 字句解析中に発生するエラーの種類。
enum class LexerErrorKind {
  kUnterminatedBlockComment,
  kUnterminatedCharacterLiteral,
  kUnterminatedStringLiteral,
  kInvalidUtf8,
  kInvalidEscapeSequence,
  kInvalidCharacterLiteralLength,
  kInvalidCharacter,
  kInvalidIntegerLiteral,
};

// 字句解析中に発生したエラーとその範囲を表す。
struct LexerError {
  LexerErrorKind kind;
  source::SourceRange range;
};

// 空白とコメントを読み飛ばした結果を表す。
struct SkipWhitespaceAndCommentsResult {
  std::size_t next_byte_offset;
  std::optional<LexerError> error;
};

// 候補なし、トークン成功、字句エラーのいずれかを表す。
using LexTokenResult = std::variant<std::monostate, Token, LexerError>;
using LexIntegerLiteralResult = LexTokenResult;
using LexQuotedLiteralResult = LexTokenResult;

// ソース全体を字句解析した結果を表す。
// UTF-8、NULの事前検証で失敗した場合、トークンは空。
// トークン生成に失敗した場合、それ以前のトークンは保持。
struct LexResult {
  std::vector<Token> tokens;
  std::optional<LexerError> error;
};

// 指定位置から連続するASCII空白とコメントを読み飛ばす。
[[nodiscard]] SkipWhitespaceAndCommentsResult SkipWhitespaceAndComments(
  std::string_view source_bytes,
  std::size_t byte_offset
);

// 指定位置から始まる識別子または予約語候補をトークンに変換する。
// 候補がなければstd::nulloptを返す。
[[nodiscard]] std::optional<Token> LexWord(
  std::string_view source_bytes,
  std::size_t byte_offset
);

// 指定位置から始まる10進整数リテラルをトークンに変換する。
// 数字から始まらなければmonostate、不正候補ならLexerErrorを返す。
[[nodiscard]] LexIntegerLiteralResult LexIntegerLiteral(
  std::string_view source_bytes,
  std::size_t byte_offset
);

// 指定位置から始まる文字・文字列リテラルをトークンに変換する。
// 引用符から始まらなければmonostateを返す。
[[nodiscard]] LexQuotedLiteralResult LexQuotedLiteral(
  std::string_view source_bytes,
  std::size_t byte_offset
);

// ソース全体のUTF-8とNULを検証し、指定位置からソース末尾まで字句解析する。
// 正常終了時、トークン末尾にEOFが入る。
[[nodiscard]] LexResult LexSource(
  std::string_view source_bytes,
  std::size_t byte_offset
);

} // namespace epp::lexer

#endif // EPP_INCLUDE_EPP_LEXER_LEXER_H_
