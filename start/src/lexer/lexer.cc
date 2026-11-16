#include "epp/lexer/lexer.h"

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

#include "epp/lexer/fixed_token.h"
#include "epp/source/source_range.h"
#include "epp/source/utf8.h"
#include "epp/unicode/x_system.h"

namespace epp::lexer {

namespace {

constexpr std::size_t kMaximumUnicodeEscapeDigitCount = 6;
constexpr char32_t kFirstSurrogate = 0xD800;
constexpr char32_t kLastSurrogate = 0xDFFF;
constexpr char32_t kMaximumUnicodeScalar = 0x10FFFF;

// エスケープシーケンスを読み取った結果を表す。
struct EscapeSequenceResult {
  std::size_t next_byte_offset;
  std::optional<char32_t> value;
  std::optional<LexerError> error;
};

// 引用符内の文字列を読み取った結果を表す。
struct QuotedLiteralContentsResult {
  std::u32string value;
  std::optional<LexerError> error;
};

// scalarが[lower, upper]の区間にあるか判定する。
bool IsInAsciiRange(char32_t scalar, char32_t lower, char32_t upper) {
  return lower <= scalar && scalar <= upper;
}

bool IsAsciiDigit(char32_t scalar) {
  return IsInAsciiRange(scalar, U'0', U'9');
}

bool IsLineBreak(char byte) {
  return byte == '\n' || byte == '\r';
}

bool IsWhitespace(char byte) {
  constexpr std::string_view kWhitespaceBytes = " \t\n\r\f";
  return kWhitespaceBytes.find(byte) != std::string_view::npos;
}

bool IsAsciiLetter(char32_t scalar) {
  return IsInAsciiRange(scalar, U'A', U'Z') || IsInAsciiRange(scalar, U'a', U'z');
}

bool IsEsperantoLetter(char32_t scalar) {
  constexpr std::u32string_view kPrecomposedEsperantoLetters = U"ĈĉĜĝĤĥĴĵŜŝŬŭ";
  return kPrecomposedEsperantoLetters.find(scalar) != std::u32string_view::npos;
}

bool IsIdentifierStart(char32_t scalar) {
  return IsAsciiLetter(scalar) || IsEsperantoLetter(scalar) || scalar == U'_';
}

bool IsIdentifierContinuation(char32_t scalar) {
  return IsIdentifierStart(scalar) || IsAsciiDigit(scalar);
}

bool IsAsciiHexDigit(char byte) {
  return IsAsciiDigit(byte) || IsInAsciiRange(byte, 'a', 'f') || IsInAsciiRange(byte, 'A', 'F');
}

char32_t HexDigitValue(char byte) {
  if (IsInAsciiRange(byte, '0', '9')) {
    return static_cast<char32_t>(byte - '0');
  }
  if (IsInAsciiRange(byte, 'a', 'f')) {
    return static_cast<char32_t>(byte - 'a' + 10);
  }
  return static_cast<char32_t>(byte - 'A' + 10);
}

std::optional<char32_t> DecodeSimpleEscape(char byte) {
  switch (byte) {
    case '\\': return U'\\';
    case '\'': return U'\'';
    case '"': return U'"';
    case 'n': return U'\n';
    case 'r': return U'\r';
    case 't': return U'\t';
    case '0': return U'\0';

    default:
      return std::nullopt;
  }
}

EscapeSequenceResult MakeInvalidEscapeSequenceResult(
  std::size_t escape_begin,
  std::size_t error_end
) {
  return EscapeSequenceResult{
    error_end,
    std::nullopt,
    LexerError{
      LexerErrorKind::kInvalidEscapeSequence,
      source::SourceRange{escape_begin, error_end}
    }
  };
}

EscapeSequenceResult DecodeEscapeSequence(
  std::string_view source_bytes,
  std::size_t escape_begin,
  std::size_t closing_quote_offset
) {
  std::size_t current_offset = escape_begin + 1;
  if (current_offset >= closing_quote_offset) {
    return MakeInvalidEscapeSequenceResult(escape_begin, current_offset);
  }

  const std::optional<char32_t> simple_value = DecodeSimpleEscape(source_bytes[current_offset]);
  if (simple_value.has_value()) {
    return EscapeSequenceResult{current_offset + 1, *simple_value, std::nullopt};
  }
  if (source_bytes[current_offset] != 'u') {
    return MakeInvalidEscapeSequenceResult(escape_begin, current_offset + 1);
  }
  ++current_offset;

  if (current_offset >= closing_quote_offset || source_bytes[current_offset] != '{') {
    const std::size_t error_end = current_offset < closing_quote_offset
      ? current_offset + 1
      : current_offset;
    return MakeInvalidEscapeSequenceResult(escape_begin, error_end);
  }
  ++current_offset;

  std::size_t digit_count = 0;
  char32_t scalar_value = 0;
  while (current_offset < closing_quote_offset && IsAsciiHexDigit(source_bytes[current_offset])) {
    if (digit_count == kMaximumUnicodeEscapeDigitCount) {
      return MakeInvalidEscapeSequenceResult(escape_begin, current_offset + 1);
    }
    scalar_value = scalar_value * 16 + HexDigitValue(source_bytes[current_offset]);
    ++digit_count;
    ++current_offset;
  }

  if (digit_count == 0 || current_offset >= closing_quote_offset || source_bytes[current_offset] != '}') {
    const std::size_t error_end = current_offset < closing_quote_offset
      ? current_offset + 1
      : current_offset;
    return MakeInvalidEscapeSequenceResult(escape_begin, error_end);
  }

  const bool is_surrogate = kFirstSurrogate <= scalar_value && scalar_value <= kLastSurrogate;
  if (is_surrogate || scalar_value > kMaximumUnicodeScalar) {
    return MakeInvalidEscapeSequenceResult(escape_begin, current_offset + 1);
  }

  return EscapeSequenceResult{current_offset + 1, scalar_value, std::nullopt};
}

QuotedLiteralContentsResult DecodeQuotedLiteralContents(
  std::string_view source_bytes,
  std::size_t opening_quote_offset,
  std::size_t closing_quote_offset,
  TokenKind token_kind
) {
  std::size_t current_offset = opening_quote_offset + 1;
  std::u32string value;
  while (current_offset < closing_quote_offset) {
    if (source_bytes[current_offset] == '\\') {
      const EscapeSequenceResult escape_result
        = DecodeEscapeSequence(source_bytes, current_offset, closing_quote_offset);
      if (escape_result.error.has_value()) {
        return QuotedLiteralContentsResult{std::u32string{}, escape_result.error};
      }
      value.push_back(*escape_result.value);
      current_offset = escape_result.next_byte_offset;
      continue;
    }

    const std::optional<source::DecodedUtf8Scalar> decoded = source::DecodeUtf8Scalar(source_bytes, current_offset);
    if (!decoded.has_value()) {
      return QuotedLiteralContentsResult{
        std::u32string{},
        LexerError{LexerErrorKind::kInvalidUtf8, source::SourceRange{current_offset, current_offset + 1}}
      };
    }

    value.push_back(decoded->value);
    current_offset += decoded->byte_count;
  }

  if (token_kind == TokenKind::kCharacterLiteral && value.size() != 1) {
    return QuotedLiteralContentsResult{
      std::u32string{},
      LexerError{
        LexerErrorKind::kInvalidCharacterLiteralLength,
        source::SourceRange{opening_quote_offset, closing_quote_offset + 1}
      }
    };
  }

  return QuotedLiteralContentsResult{std::move(value), std::nullopt};
}

} // namespace

SkipWhitespaceAndCommentsResult SkipWhitespaceAndComments(
  std::string_view source_bytes,
  std::size_t byte_offset
) {
  if (byte_offset >= source_bytes.size()) {
    return SkipWhitespaceAndCommentsResult{
      source_bytes.size(),
      std::nullopt
    };
  }
  std::size_t current_offset = byte_offset;
  while (current_offset < source_bytes.size()) {
    const std::string_view remaining = source_bytes.substr(current_offset);

    if (IsWhitespace(source_bytes[current_offset])) {
      ++current_offset;
      continue;
    }

    if (remaining.starts_with("//")) {
      current_offset += 2;
      while (current_offset < source_bytes.size() && !IsLineBreak(source_bytes[current_offset])) {
        ++current_offset;
      }
      continue;
    }

    if (remaining.starts_with("/*")) {
      const std::size_t comment_begin = current_offset;
      const std::size_t comment_end = source_bytes.find("*/", current_offset + 2);
      if (comment_end == std::string_view::npos) {
        return SkipWhitespaceAndCommentsResult{
          source_bytes.size(),
          LexerError{
            LexerErrorKind::kUnterminatedBlockComment,
            source::SourceRange{comment_begin, source_bytes.size()}
          }
        };
      }
      current_offset = comment_end + 2;
      continue;
    }

    break;
  }

  return SkipWhitespaceAndCommentsResult{current_offset, std::nullopt};
}

} // namespace epp::lexer
