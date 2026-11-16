#ifndef BOOK_TEST_H_
#define BOOK_TEST_H_

#include <cstdint>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <variant>

#include "epp/lexer/lexer.h"

// 表示専用の関数であり、字句解析は行わない。
namespace book {

inline std::string Utf8(std::u32string_view text) {
  std::string bytes;
  for (const char32_t scalar : text) {
    const auto value = static_cast<std::uint32_t>(scalar);
    if (value <= 0x7f) {
      bytes.push_back(static_cast<char>(value));
    } else if (value <= 0x7ff) {
      bytes.push_back(static_cast<char>(0xc0 | (value >> 6)));
      bytes.push_back(static_cast<char>(0x80 | (value & 0x3f)));
    } else if (value <= 0xffff) {
      bytes.push_back(static_cast<char>(0xe0 | (value >> 12)));
      bytes.push_back(static_cast<char>(0x80 | ((value >> 6) & 0x3f)));
      bytes.push_back(static_cast<char>(0x80 | (value & 0x3f)));
    } else {
      bytes.push_back(static_cast<char>(0xf0 | (value >> 18)));
      bytes.push_back(static_cast<char>(0x80 | ((value >> 12) & 0x3f)));
      bytes.push_back(static_cast<char>(0x80 | ((value >> 6) & 0x3f)));
      bytes.push_back(static_cast<char>(0x80 | (value & 0x3f)));
    }
  }
  return bytes;
}

inline std::string Escaped(std::u32string_view text, char quote) {
  std::ostringstream out;
  out << quote;
  for (const char32_t scalar : text) {
    switch (scalar) {
      case U'\n':
        out << "\\n";
        break;
      case U'\r':
        out << "\\r";
        break;
      case U'\t':
        out << "\\t";
        break;
      case U'\0':
        out << "\\0";
        break;
      case U'\\':
        out << "\\\\";
        break;
      default:
        if (scalar == static_cast<char32_t>(quote)) {
          out << '\\';
        }
        if (scalar < 0x20 || scalar == 0x7f) {
          out << "\\u{" << std::hex << static_cast<std::uint32_t>(scalar) << '}';
        } else {
          out << Utf8(std::u32string_view(&scalar, 1));
        }
    }
  }
  out << quote;
  return out.str();
}

inline std::string_view KindName(epp::lexer::TokenKind kind) {
  using epp::lexer::TokenKind;
  switch (kind) {
    case TokenKind::kEndOfFile:
      return "kEndOfFile";
    case TokenKind::kIdentifier:
      return "kIdentifier";
    case TokenKind::kIntegerLiteral:
      return "kIntegerLiteral";
    case TokenKind::kTrueLiteral:
      return "kTrueLiteral";
    case TokenKind::kFalseLiteral:
      return "kFalseLiteral";
    case TokenKind::kCharacterLiteral:
      return "kCharacterLiteral";
    case TokenKind::kStringLiteral:
      return "kStringLiteral";
    case TokenKind::kReserved:
      return "kReserved";
    case TokenKind::kElse:
      return "kElse";
    case TokenKind::kContinue:
      return "kContinue";
    case TokenKind::kWhile:
      return "kWhile";
    case TokenKind::kConst:
      return "kConst";
    case TokenKind::kReturn:
      return "kReturn";
    case TokenKind::kBreak:
      return "kBreak";
    case TokenKind::kIf:
      return "kIf";
    case TokenKind::kVoid:
      return "kVoid";
    case TokenKind::kOr:
      return "kOr";
    case TokenKind::kAnd:
      return "kAnd";
    case TokenKind::kNot:
      return "kNot";
    case TokenKind::kLeftParenthesis:
      return "kLeftParenthesis";
    case TokenKind::kRightParenthesis:
      return "kRightParenthesis";
    case TokenKind::kLeftBrace:
      return "kLeftBrace";
    case TokenKind::kRightBrace:
      return "kRightBrace";
    case TokenKind::kComma:
      return "kComma";
    case TokenKind::kSemicolon:
      return "kSemicolon";
    case TokenKind::kPlus:
      return "kPlus";
    case TokenKind::kMinus:
      return "kMinus";
    case TokenKind::kStar:
      return "kStar";
    case TokenKind::kSlash:
      return "kSlash";
    case TokenKind::kPercent:
      return "kPercent";
    case TokenKind::kTilde:
      return "kTilde";
    case TokenKind::kPipe:
      return "kPipe";
    case TokenKind::kCaret:
      return "kCaret";
    case TokenKind::kAmpersand:
      return "kAmpersand";
    case TokenKind::kShiftLeft:
      return "kShiftLeft";
    case TokenKind::kShiftRight:
      return "kShiftRight";
    case TokenKind::kLess:
      return "kLess";
    case TokenKind::kLessEqual:
      return "kLessEqual";
    case TokenKind::kGreater:
      return "kGreater";
    case TokenKind::kGreaterEqual:
      return "kGreaterEqual";
    case TokenKind::kEqualEqual:
      return "kEqualEqual";
    case TokenKind::kBangEqual:
      return "kBangEqual";
    case TokenKind::kEqual:
      return "kEqual";
  }
  return "unknown";
}

inline std::string_view ErrorName(epp::lexer::LexerErrorKind kind) {
  using epp::lexer::LexerErrorKind;
  switch (kind) {
    case LexerErrorKind::kUnterminatedBlockComment:
      return "kUnterminatedBlockComment";
    case LexerErrorKind::kUnterminatedCharacterLiteral:
      return "kUnterminatedCharacterLiteral";
    case LexerErrorKind::kUnterminatedStringLiteral:
      return "kUnterminatedStringLiteral";
    case LexerErrorKind::kInvalidUtf8:
      return "kInvalidUtf8";
    case LexerErrorKind::kInvalidEscapeSequence:
      return "kInvalidEscapeSequence";
    case LexerErrorKind::kInvalidCharacterLiteralLength:
      return "kInvalidCharacterLiteralLength";
    case LexerErrorKind::kInvalidCharacter:
      return "kInvalidCharacter";
    case LexerErrorKind::kInvalidIntegerLiteral:
      return "kInvalidIntegerLiteral";
  }
  return "unknown";
}

inline void PrintName(std::u32string_view name) {
  std::cout << Utf8(name) << '\n';
}

inline void PrintToken(const epp::lexer::Token& token) {
  using namespace epp::lexer;
  std::cout
    << KindName(token.kind) << " ["
    << token.range.begin_byte_offset << ", "
    << token.range.end_byte_offset << ')';
  if (token.kind == TokenKind::kIdentifier) {
    std::cout << " name=" << Escaped(token.normalized_identifier, '"');
  }
  if (const auto* integer = std::get_if<IntegerLiteralTokenValue>(&token.literal_value)) {
    std::cout << " digits=" << std::quoted(integer->digits);
  } else if (const auto* character = std::get_if<char32_t>(&token.literal_value)) {
    std::cout << " value=" << Escaped(std::u32string_view(character, 1), '\'');
  } else if (const auto* string = std::get_if<std::u32string>(&token.literal_value)) {
    std::cout << " value=" << Escaped(*string, '"');
  }
  std::cout << '\n';
}

inline int PrintError(const epp::lexer::LexerError& error) {
  std::cerr
    << ErrorName(error.kind) << " ["
    << error.range.begin_byte_offset << ", "
    << error.range.end_byte_offset << ")\n";
  return 1;
}

inline int PrintReadResult(const epp::lexer::LexTokenResult& result) {
  if (const auto* token = std::get_if<epp::lexer::Token>(&result)) {
    PrintToken(*token);
    return 0;
  }
  if (const auto* error = std::get_if<epp::lexer::LexerError>(&result)) {
    return PrintError(*error);
  }
  std::cout << "no candidate\n";
  return 0;
}

inline int PrintLexResult(const epp::lexer::LexResult& result) {
  for (const auto& token : result.tokens) {
    PrintToken(token);
  }
  if (result.error.has_value()) {
    std::cout.flush();
    return PrintError(*result.error);
  }
  return 0;
}

} // namespace book
#endif
