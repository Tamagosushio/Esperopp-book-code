#ifndef EPP_INCLUDE_EPP_PARSER_PARSER_H_
#define EPP_INCLUDE_EPP_PARSER_PARSER_H_

#include <cstddef>
#include <variant>
#include <vector>

#include "epp/ast/node.h"
#include "epp/lexer/token.h"
#include "epp/source/source_range.h"

namespace epp::parser {

// 構文解析中に発生するエラーの種類。
enum class ParserErrorKind {
  // 式が必要な位置で式を解析できない。
  kExpectedExpression,
  // 左括弧が必要な位置に存在しない。
  kExpectedLeftParenthesis,
  // 右括弧が必要な位置に存在しない。
  kExpectedRightParenthesis,
  // コンマまたは右括弧が必要な位置に、どちらも存在しない。
  kExpectedCommaOrRightParenthesis,
  // 代入先が必要な位置で識別子を解析できない。
  kExpectedAssignmentTarget,
  // 代入演算子が必要な位置に存在しない。
  kExpectedAssignmentOperator,
  // セミコロンが必要な位置に存在しない。
  kExpectedSemicolon,
  // 関数呼び出し文が必要な位置で呼び出し式を解析できない。
  kExpectedCallExpression,
  // liveru文が必要な位置にliveruキーワードが存在しない。
  kExpectedReturnKeyword,
  // 左波括弧が必要な位置に存在しない。
  kExpectedLeftBrace,
  // 右波括弧が必要な位置に存在しない。
  kExpectedRightBrace,
  // 文が必要な位置で文を解析できない。
  kExpectedStatement,
  // 宣言要素が必要な位置に、識別子または修飾子が存在しない。
  kExpectedDeclarationComponent,
  // 等号またはセミコロンが必要な位置に、どちらも存在しない。
  kExpectedEqualOrSemicolon,
  // se文が必要な位置にseキーワードが存在しない。
  kExpectedIfKeyword,
  // dum文が必要な位置にdumキーワードが存在しない。
  kExpectedWhileKeyword,
  // rompu文が必要な位置にrompuキーワードが存在しない。
  kExpectedBreakKeyword,
  // daŭrigu文が必要な位置にdaŭriguキーワードが存在しない。
  kExpectedContinueKeyword,
  // 関数ヘッダの構成要素が必要な位置に、識別子またはvakaが存在しない。
  // 戻り値の型と関数名の役割は意味解析で検査する。
  kExpectedFunctionHeadComponent,
};

// 構文解析中に発生したエラーと、その検出範囲を表す。
struct ParserError {
  ParserErrorKind kind;
  source::SourceRange range;
};

// 成功したノード番号か、検出範囲を持つ構文エラー。
using ParseResult = std::variant<ast::NodeId, ParserError>;

// トークン列の現在位置を管理する。
// tokensはカーソルより長く生存させ、解析中には変更しない。
class TokenCursor {
 public:
  explicit TokenCursor(const std::vector<lexer::Token>& tokens);
  // 現在位置のトークンを返す。
  [[nodiscard]] const lexer::Token& CurrentToken() const;
  // 現在位置が終了位置(EOF)か判定する。
  [[nodiscard]] bool IsAtEnd() const;
  // 現在位置から指定された数だけ先のトークンを返す。
  // EOFより先を指定した場合はEOFトークンを返す。
  [[nodiscard]] const lexer::Token& PeekToken(std::size_t lookahead) const;
  // 現在位置を次のトークンに進める。
  void Advance();

 private:
  // トークン列の参照を持つ。
  const std::vector<lexer::Token>& tokens_;
  // tokens_の添え字。
  std::size_t current_token_index_ = 0;
};

// 現在位置から式を解析する。
[[nodiscard]] ParseResult ParseExpression(
  TokenCursor& cursor,
  ast::Ast& ast
);

// 現在位置からローカル宣言を解析する。
// 成功した場合、セミコロンの直後まで現在位置を進める。
[[nodiscard]] ParseResult ParseLocalDeclarationStatement(
  TokenCursor& cursor,
  ast::Ast& ast
);

// 現在位置から代入文を解析する。
// 成功した場合、セミコロンの直後まで現在位置を進める。
[[nodiscard]] ParseResult ParseAssignmentStatement(
  TokenCursor& cursor,
  ast::Ast& ast
);

// 現在位置からliveru文を解析する。
// 成功した場合、セミコロンの直後まで現在位置を進める。
[[nodiscard]] ParseResult ParseReturnStatement(
  TokenCursor& cursor,
  ast::Ast& ast
);

// 現在位置のトークンから対応する文を選んで解析する。
[[nodiscard]] ParseResult ParseStatement(
  TokenCursor& cursor,
  ast::Ast& ast
);

// 現在位置から文ブロックを解析する。
// 成功した場合、右波括弧の直後まで現在位置を進める。
[[nodiscard]] ParseResult ParseBlockStatement(
  TokenCursor& cursor,
  ast::Ast& ast
);

// 現在位置から関数定義を解析する。
// 成功した場合、関数本体の直後まで現在位置を進める。
[[nodiscard]] ParseResult ParseFunctionDefinition(
  TokenCursor& cursor,
  ast::Ast& ast
);

// 現在位置からコンパイル単位を解析する。
// 成功した場合、現在位置はEOFになる。
[[nodiscard]] ParseResult ParseCompilationUnit(
  TokenCursor& cursor,
  ast::Ast& ast
);

} // namespace epp::parser

#endif // EPP_INCLUDE_EPP_PARSER_PARSER_H_
