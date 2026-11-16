#include "epp/lexer/token.h"

#include <string_view>

namespace epp::lexer {

namespace {

struct WordTokenKind {
  std::u32string_view spelling;
  TokenKind kind;
};

constexpr WordTokenKind kWordTokenKinds[] = {
  {U"alie", TokenKind::kElse},
  {U"aŭ", TokenKind::kOr},
  {U"aŭtomata", TokenKind::kReserved},
  {U"daŭrigu", TokenKind::kContinue},
  {U"dum", TokenKind::kWhile},
  {U"en", TokenKind::kReserved},
  {U"kaj", TokenKind::kAnd},
  {U"konstanta", TokenKind::kConst},
  {U"konstantesprima", TokenKind::kReserved},
  {U"liveru", TokenKind::kReturn},
  {U"malvera", TokenKind::kFalseLiteral},
  {U"ne", TokenKind::kNot},
  {U"por", TokenKind::kReserved},
  {U"rompu", TokenKind::kBreak},
  {U"se", TokenKind::kIf},
  {U"strukturo", TokenKind::kReserved},
  {U"vaka", TokenKind::kVoid},
  {U"vera", TokenKind::kTrueLiteral},
};

} // namespace

TokenKind ClassifyNormalizedWord(
  std::u32string_view normalized_word
) noexcept {
  for (const WordTokenKind& word_token_kind : kWordTokenKinds) {
    if (normalized_word == word_token_kind.spelling) {
      return word_token_kind.kind;
    }
  }
  return TokenKind::kIdentifier;
}

} // namespace epp::lexer
