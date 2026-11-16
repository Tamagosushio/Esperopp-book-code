#ifndef EPP_INCLUDE_EPP_LEXER_TOKEN_H_
#define EPP_INCLUDE_EPP_LEXER_TOKEN_H_

#include <string>
#include <string_view>
#include <variant>

#include "epp/source/source_range.h"

namespace epp::lexer {

// 字句解析で識別したトークンの種類を表す。
enum class TokenKind {
  // ソースの終端
  kEndOfFile,
  // 名前とリテラル
  kIdentifier,
  kIntegerLiteral,
  kTrueLiteral,
  kFalseLiteral,
  kCharacterLiteral,
  kStringLiteral,
  // 使用を禁止する予約語・演算子。
  // 識別子とは区別する。
  kReserved,
  // 予約語
  kElse,
  kContinue,
  kWhile,
  kConst,
  kReturn,
  kBreak,
  kIf,
  kVoid,
  // 論理演算子
  kOr,
  kAnd,
  kNot,
  // 括弧と区切り記号
  kLeftParenthesis,
  kRightParenthesis,
  kLeftBrace,
  kRightBrace,
  kComma,
  kSemicolon,
  // 算術演算子
  kPlus,
  kMinus,
  kStar,
  kSlash,
  kPercent,
  // ビット演算とシフト
  kTilde,
  kPipe,
  kCaret,
  kAmpersand,
  kShiftLeft,
  kShiftRight,
  // 比較演算子
  kLess,
  kLessEqual,
  kGreater,
  kGreaterEqual,
  kEqualEqual,
  kBangEqual,
  // 代入演算子
  kEqual,
};

// x-system変換後の単語をトークンの種類へ分類する。
[[nodiscard]] TokenKind ClassifyNormalizedWord(
  std::u32string_view normalized_word
) noexcept;

} // namespace epp::lexer

#endif // EPP_INCLUDE_EPP_LEXER_TOKEN_H_
