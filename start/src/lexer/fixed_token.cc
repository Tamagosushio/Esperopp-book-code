#include "epp/lexer/fixed_token.h"

#include <cstddef>
#include <optional>
#include <string_view>

#include "epp/lexer/token.h"
#include "epp/source/source_range.h"

namespace epp::lexer {

namespace {

struct FixedTokenSpelling {
  std::string_view spelling;
  TokenKind kind;
};

// 順に走査するため、文字数の降順に並べる。
constexpr FixedTokenSpelling kFixedTokenSpellings[] = {
  {"++", TokenKind::kReserved},
  {"--", TokenKind::kReserved},
  {"+=", TokenKind::kReserved},
  {"-=", TokenKind::kReserved},
  {"*=", TokenKind::kReserved},
  {"/=", TokenKind::kReserved},
  {"%=", TokenKind::kReserved},
  {"<=", TokenKind::kLessEqual},
  {">=", TokenKind::kGreaterEqual},
  {"==", TokenKind::kEqualEqual},
  {"!=", TokenKind::kBangEqual},
  {"<<", TokenKind::kShiftLeft},
  {">>", TokenKind::kShiftRight},

  {"(", TokenKind::kLeftParenthesis},
  {")", TokenKind::kRightParenthesis},
  {"{", TokenKind::kLeftBrace},
  {"}", TokenKind::kRightBrace},
  {",", TokenKind::kComma},
  {";", TokenKind::kSemicolon},
  {"+", TokenKind::kPlus},
  {"-", TokenKind::kMinus},
  {"*", TokenKind::kStar},
  {"/", TokenKind::kSlash},
  {"%", TokenKind::kPercent},
  {"~", TokenKind::kTilde},
  {"|", TokenKind::kPipe},
  {"^", TokenKind::kCaret},
  {"&", TokenKind::kAmpersand},
  {"<", TokenKind::kLess},
  {">", TokenKind::kGreater},
  {"=", TokenKind::kEqual},
};

} // namespace

} // namespace epp::lexer
