#include "epp/parser/parser.h"

#include <stdexcept>
#include <optional>
#include <string>
#include <variant>

namespace epp::parser {

TokenCursor::TokenCursor(const std::vector<lexer::Token>& tokens)
  : tokens_(tokens) {
}

const lexer::Token& TokenCursor::CurrentToken() const {
  return tokens_.at(current_token_index_);
}

const lexer::Token& TokenCursor::PeekToken(std::size_t lookahead) const {
  if (lookahead >= tokens_.size() - current_token_index_) {
    return tokens_.back();
  }
  return tokens_.at(current_token_index_ + lookahead);
}

bool TokenCursor::IsAtEnd() const {
  return CurrentToken().kind == lexer::TokenKind::kEndOfFile;
}

void TokenCursor::Advance() {
  if (!IsAtEnd()) {
    ++current_token_index_;
  }
}

namespace {

// 構文エラーだけを公開入口で結果へ戻す。内部例外は捕捉しない。
struct ParseFailure {
  ParserError error;
};

[[noreturn]] void Fail(ParserErrorKind kind, source::SourceRange range) {
  throw ParseFailure{{kind, range}};
}

// 必須トークンを消費し、前進する前の診断範囲を返す。
source::SourceRange Consume(
  TokenCursor& cursor,
  lexer::TokenKind kind,
  ParserErrorKind error
) {
  const auto range = cursor.CurrentToken().range;
  if (cursor.CurrentToken().kind != kind) {
    Fail(error, range);
  }
  cursor.Advance();
  return range;
}

template <ast::NodeId (*Parse)(TokenCursor&, ast::Ast&)>
ParseResult AtEntry(TokenCursor& cursor, ast::Ast& ast) {
  try {
    return Parse(cursor, ast);
  } catch (const ParseFailure& failure) {
    return failure.error;
  }
}

ast::NodeId ParseExpressionImpl(TokenCursor&, ast::Ast&);
ast::NodeId ParseBlockStatementImpl(TokenCursor&, ast::Ast&);

std::optional<ast::NodeId> ParseLiteral(
  const lexer::Token& token,
  ast::Ast& ast
) {
  switch (token.kind) {
    case lexer::TokenKind::kIntegerLiteral: {
      const auto* value = std::get_if<lexer::IntegerLiteralTokenValue>(&token.literal_value);
      if (value == nullptr) {
        return std::nullopt;
      }
      return ast.AddNode(ast::Node{
        token.range,
        ast::IntegerLiteralExpression{value->digits}
      });
    }

    case lexer::TokenKind::kTrueLiteral:
      return ast.AddNode(ast::Node{token.range, ast::BooleanLiteralExpression{true}});

    case lexer::TokenKind::kFalseLiteral:
      return ast.AddNode(ast::Node{token.range, ast::BooleanLiteralExpression{false}});

    case lexer::TokenKind::kCharacterLiteral: {
      const auto* value = std::get_if<char32_t>(&token.literal_value);
      if (value == nullptr) {
        return std::nullopt;
      }
      return ast.AddNode(ast::Node{token.range, ast::CharacterLiteralExpression{*value}});
    }

    case lexer::TokenKind::kStringLiteral: {
      const auto* value = std::get_if<std::u32string>(&token.literal_value);
      if (value == nullptr) {
        return std::nullopt;
      }
      return ast.AddNode(ast::Node{token.range, ast::StringLiteralExpression{*value}});
    }

    default:
      return std::nullopt;
  }
}

std::optional<ast::NodeId> ParseIdentifier(
  const lexer::Token& token,
  ast::Ast& ast
) {
  if (token.kind != lexer::TokenKind::kIdentifier) {
    return std::nullopt;
  }
  return ast.AddNode(ast::Node{
    token.range,
    ast::NameExpression{token.normalized_identifier}
  });
}

// 括弧自体はASTへ追加せず、中の式とその診断位置を保つ。
ast::NodeId ParseParenthesizedExpression(
  TokenCursor& cursor,
  ast::Ast& ast
) {
  Consume(
    cursor,
    lexer::TokenKind::kLeftParenthesis,
    ParserErrorKind::kExpectedLeftParenthesis
  );
  const auto expression_id = ParseExpressionImpl(cursor, ast);
  Consume(
    cursor,
    lexer::TokenKind::kRightParenthesis,
    ParserErrorKind::kExpectedRightParenthesis
  );
  return expression_id;
}

ast::NodeId FinishCallExpression(
  TokenCursor& cursor,
  ast::Ast& ast,
  ast::NodeId callee_id
) {
  const std::size_t begin_byte_offset = ast.GetNode(callee_id).range.begin_byte_offset;
  cursor.Advance();

  std::vector<ast::NodeId> arguments;
  // 右括弧でなければ、引数が存在する。
  if (cursor.CurrentToken().kind != lexer::TokenKind::kRightParenthesis) {
    while (true) {
      ast::NodeId argument_id = ParseExpressionImpl(cursor, ast);

      arguments.push_back(argument_id);
      // 右括弧なら引数の解析を終了する。
      if (cursor.CurrentToken().kind == lexer::TokenKind::kRightParenthesis) {
        break;
      }
      // 右括弧でなければコンマが必要。
      if (cursor.CurrentToken().kind != lexer::TokenKind::kComma) {
        Fail(
          ParserErrorKind::kExpectedCommaOrRightParenthesis,
          cursor.CurrentToken().range
        );
      }
      cursor.Advance();
    }
  }

  const std::size_t end_byte_offset = cursor.CurrentToken().range.end_byte_offset;
  cursor.Advance();
  const ast::NodeId call_id = ast.AddNode(ast::Node{
    source::SourceRange{begin_byte_offset, end_byte_offset},
    ast::CallExpression{callee_id, arguments}
  });

  return call_id;
}

ast::NodeId ParsePrimaryExpression(
  TokenCursor& cursor,
  ast::Ast& ast
) {
  const lexer::Token& current_token = cursor.CurrentToken();
  // 左括弧から始まる場合、括弧内の式を解析する。
  if (current_token.kind == lexer::TokenKind::kLeftParenthesis) {
    return ParseParenthesizedExpression(cursor, ast);
  }

  std::optional<ast::NodeId> literal_id = ParseLiteral(current_token, ast);
  if (literal_id.has_value()) {
    cursor.Advance();
    return *literal_id;
  }

  const std::optional<ast::NodeId> name_id = ParseIdentifier(current_token, ast);
  if (!name_id.has_value()) {
    Fail(
      ParserErrorKind::kExpectedExpression,
      current_token.range
    );
  }

  cursor.Advance();

  // 識別子の直後が左括弧なら、呼び出し式として解析する。
  if (cursor.CurrentToken().kind == lexer::TokenKind::kLeftParenthesis) {
    return FinishCallExpression(cursor, ast, *name_id);
  }
  return *name_id;
}

// トークンの種類をASTの単項演算子に変換する。
// 単項演算子でなければstd::nulloptを返す。
std::optional<ast::UnaryOperator> ToUnaryOperator(lexer::TokenKind token_kind) {
  switch (token_kind) {
    case lexer::TokenKind::kPlus:
      return ast::UnaryOperator::kPlus;
    case lexer::TokenKind::kMinus:
      return ast::UnaryOperator::kMinus;
    case lexer::TokenKind::kTilde:
      return ast::UnaryOperator::kBitwiseNot;
    case lexer::TokenKind::kNot:
      return ast::UnaryOperator::kLogicalNot;

    default:
      return std::nullopt;
  }
}

ast::NodeId ParseUnaryExpression(
  TokenCursor& cursor,
  ast::Ast& ast
) {
  const lexer::Token& operator_token = cursor.CurrentToken();
  const std::optional<ast::UnaryOperator> operator_kind = ToUnaryOperator(operator_token.kind);
  // 単項演算子でなければ基本式を解析する。
  if (!operator_kind.has_value()) {
    return ParsePrimaryExpression(cursor, ast);
  }
  cursor.Advance();
  ast::NodeId operand_id = ParseUnaryExpression(cursor, ast);

  const ast::Node& operand_node = ast.GetNode(operand_id);
  const ast::NodeId unary_id = ast.AddNode(
    ast::Node{
      source::SourceRange{
        operator_token.range.begin_byte_offset,
        operand_node.range.end_byte_offset},
      ast::UnaryExpression{*operator_kind, operand_id}}
  );

  return unary_id;
}

struct BinaryOperatorInfo {
  lexer::TokenKind token;
  ast::BinaryOperator operation;
  int precedence;
};
constexpr BinaryOperatorInfo kBinaryOperators[] = {
  {lexer::TokenKind::kOr, ast::BinaryOperator::kLogicalOr, 1},
  {lexer::TokenKind::kAnd, ast::BinaryOperator::kLogicalAnd, 2},
  {lexer::TokenKind::kPipe, ast::BinaryOperator::kBitwiseOr, 3},
  {lexer::TokenKind::kCaret, ast::BinaryOperator::kBitwiseXor, 4},
  {lexer::TokenKind::kAmpersand, ast::BinaryOperator::kBitwiseAnd, 5},
  {lexer::TokenKind::kEqualEqual, ast::BinaryOperator::kEqual, 6},
  {lexer::TokenKind::kBangEqual, ast::BinaryOperator::kNotEqual, 6},
  {lexer::TokenKind::kLess, ast::BinaryOperator::kLess, 7},
  {lexer::TokenKind::kLessEqual, ast::BinaryOperator::kLessEqual, 7},
  {lexer::TokenKind::kGreater, ast::BinaryOperator::kGreater, 7},
  {lexer::TokenKind::kGreaterEqual, ast::BinaryOperator::kGreaterEqual, 7},
  {lexer::TokenKind::kShiftLeft, ast::BinaryOperator::kShiftLeft, 8},
  {lexer::TokenKind::kShiftRight, ast::BinaryOperator::kShiftRight, 8},
  {lexer::TokenKind::kPlus, ast::BinaryOperator::kAdd, 9},
  {lexer::TokenKind::kMinus, ast::BinaryOperator::kSubtract, 9},
  {lexer::TokenKind::kStar, ast::BinaryOperator::kMultiply, 10},
  {lexer::TokenKind::kSlash, ast::BinaryOperator::kDivide, 10},
  {lexer::TokenKind::kPercent, ast::BinaryOperator::kRemainder, 10},
};

ast::NodeId ParseBinaryExpression(
  TokenCursor& cursor,
  ast::Ast& ast,
  int minimum = 1
) {
  throw std::logic_error("not yet implemented: ParseBinaryExpression");
}

ast::NodeId ParseExpressionImpl(
  TokenCursor& cursor,
  ast::Ast& ast
) {
  return ParseBinaryExpression(
    cursor, ast
  );
}

// トークンを宣言修飾子に変換する。
// 宣言修飾子でなければstd::nulloptを返す。
std::optional<ast::DeclarationModifier> ToDeclarationModifier(const lexer::TokenKind token_kind) {
  switch (token_kind) {
    case lexer::TokenKind::kConst:
      return ast::DeclarationModifier::kConst;

    default:
      return std::nullopt;
  }
}

// トークンを宣言の構成要素に変換する。
// 宣言の構成要素でなければstd::nulloptを返す。
std::optional<ast::DeclarationComponent> ToDeclarationComponent(const lexer::Token& token) {
  if (token.kind == lexer::TokenKind::kIdentifier) {
    return ast::DeclarationComponent{
      token.range,
      ast::DeclarationIdentifier{token.normalized_identifier}
    };
  }
  const std::optional<ast::DeclarationModifier> modifier = ToDeclarationModifier(token.kind);
  if (modifier.has_value()) {
    return ast::DeclarationComponent{token.range, *modifier};
  }

  return std::nullopt;
}

// 現在位置から宣言の構成要素を最大3個読み取る。
std::vector<ast::DeclarationComponent> ParseDeclarationComponents(TokenCursor& cursor) {
  std::vector<ast::DeclarationComponent> components;
  while (components.size() < 3) {
    const std::optional<ast::DeclarationComponent> component = ToDeclarationComponent(cursor.CurrentToken());
    if (!component.has_value()) {
      break;
    }
    components.push_back(*component);
    cursor.Advance();
  }
  return components;
}

// セミコロンで終わるローカル宣言を解析する。
ast::NodeId ParseLocalDeclarationStatementImpl(
  TokenCursor& cursor,
  ast::Ast& ast
) {
  throw std::logic_error("not yet implemented: ParseLocalDeclarationStatementImpl");
}

ast::NodeId ParseAssignmentStatementImpl(TokenCursor& cursor, ast::Ast& ast) {
  const lexer::Token& target_token = cursor.CurrentToken();
  const std::optional<ast::NodeId> target_id = ParseIdentifier(target_token, ast);
  if (!target_id.has_value()) {
    Fail(ParserErrorKind::kExpectedAssignmentTarget, target_token.range);
  }
  cursor.Advance();
  Consume(
    cursor,
    lexer::TokenKind::kEqual,
    ParserErrorKind::kExpectedAssignmentOperator
  );
  ast::NodeId value_expression_id = ParseExpressionImpl(cursor, ast);

  const auto end_byte_offset = Consume(
    cursor,
    lexer::TokenKind::kSemicolon,
    ParserErrorKind::kExpectedSemicolon
  ).end_byte_offset;

  const ast::NodeId assignment_id = ast.AddNode(ast::Node{
    source::SourceRange{target_token.range.begin_byte_offset, end_byte_offset},
    ast::AssignmentStatement{*target_id, value_expression_id}
  });

  return assignment_id;
}

ast::NodeId ParseReturnStatementImpl(
  TokenCursor& cursor,
  ast::Ast& ast
) {
  const lexer::Token& return_token = cursor.CurrentToken();
  Consume(
    cursor,
    lexer::TokenKind::kReturn,
    ParserErrorKind::kExpectedReturnKeyword
  );

  std::optional<ast::NodeId> value_id;
  // セミコロンでなければ戻り値を解析する。
  if (cursor.CurrentToken().kind != lexer::TokenKind::kSemicolon) {
    if (cursor.CurrentToken().kind == lexer::TokenKind::kEndOfFile) {
      Fail(ParserErrorKind::kExpectedSemicolon, cursor.CurrentToken().range);
    }
    ast::NodeId value_expression_id = ParseExpressionImpl(cursor, ast);

    value_id = value_expression_id;
  }

  const auto end_byte_offset = Consume(
    cursor,
    lexer::TokenKind::kSemicolon,
    ParserErrorKind::kExpectedSemicolon
  ).end_byte_offset;

  const ast::NodeId return_id = ast.AddNode(ast::Node{
    source::SourceRange{return_token.range.begin_byte_offset, end_byte_offset},
    ast::ReturnStatement{value_id}
  });

  return return_id;
}

ast::NodeId ParseCallStatement(
  TokenCursor& cursor,
  ast::Ast& ast
) {
  ast::NodeId call_id = ParsePrimaryExpression(cursor, ast);

  // 呼び出し文にはCallExpressionが必要。
  const ast::Node& call_node = ast.GetNode(call_id);
  if (!std::holds_alternative<ast::CallExpression>(call_node.data)) {
    Fail(ParserErrorKind::kExpectedCallExpression, call_node.range);
  }
  Consume(
    cursor,
    lexer::TokenKind::kSemicolon,
    ParserErrorKind::kExpectedSemicolon
  );
  return call_id;
}

// keywordとセミコロンから、子ノードを持たないループ制御文を解析する。
ast::NodeId ParseLoopControlStatement(
  TokenCursor& cursor,
  ast::Ast& ast,
  lexer::TokenKind expected_keyword_kind,
  ParserErrorKind expected_keyword_error,
  ast::NodeData statement_data
) {
  const lexer::Token& keyword_token = cursor.CurrentToken();
  if (keyword_token.kind != expected_keyword_kind) {
    Fail(expected_keyword_error, keyword_token.range);
  }
  cursor.Advance();

  const auto end_byte_offset = Consume(
    cursor,
    lexer::TokenKind::kSemicolon,
    ParserErrorKind::kExpectedSemicolon
  ).end_byte_offset;

  const ast::NodeId statement_id = ast.AddNode(ast::Node{
    source::SourceRange{keyword_token.range.begin_byte_offset, end_byte_offset},
    statement_data
  });

  return statement_id;
}

ast::NodeId ParseBreakStatement(TokenCursor& cursor, ast::Ast& ast) {
  return ParseLoopControlStatement(
    cursor,
    ast,
    lexer::TokenKind::kBreak,
    ParserErrorKind::kExpectedBreakKeyword,
    ast::BreakStatement{}
  );
}

ast::NodeId ParseContinueStatement(TokenCursor& cursor, ast::Ast& ast) {
  return ParseLoopControlStatement(
    cursor,
    ast,
    lexer::TokenKind::kContinue,
    ParserErrorKind::kExpectedContinueKeyword,
    ast::ContinueStatement{}
  );
}

ast::NodeId ParseIfStatement(TokenCursor& cursor, ast::Ast& ast) {
  const lexer::Token& if_token = cursor.CurrentToken();
  if (if_token.kind != lexer::TokenKind::kIf) {
    Fail(ParserErrorKind::kExpectedIfKeyword, if_token.range);
  }
  const std::size_t begin_byte_offset = if_token.range.begin_byte_offset;

  cursor.Advance();

  std::vector<ast::IfBranch> branches;
  std::optional<ast::NodeId> else_body_id;
  std::size_t end_byte_offset = begin_byte_offset;

  while (true) {
    const auto condition_id = ParseParenthesizedExpression(cursor, ast);
    ast::NodeId body_id = ParseBlockStatementImpl(cursor, ast);

    branches.push_back(ast::IfBranch{condition_id, body_id});
    end_byte_offset = ast.GetNode(body_id).range.end_byte_offset;
    if (cursor.CurrentToken().kind != lexer::TokenKind::kElse) {
      break;
    }

    cursor.Advance();

    // alie seなら、次の条件分岐を解析する。
    if (cursor.CurrentToken().kind == lexer::TokenKind::kIf) {
      cursor.Advance();
      continue;
    }
    // alieの直後にseがなければ、最後の文ブロックを解析する。
    ast::NodeId else_block_id = ParseBlockStatementImpl(cursor, ast);

    else_body_id = else_block_id;
    end_byte_offset = ast.GetNode(*else_body_id).range.end_byte_offset;
    break;
  }

  const ast::NodeId if_id = ast.AddNode(ast::Node{
    source::SourceRange{begin_byte_offset, end_byte_offset},
    ast::IfStatement{branches, else_body_id}
  });

  return if_id;
}

ast::NodeId ParseWhileStatement(TokenCursor& cursor, ast::Ast& ast) {
  const lexer::Token& while_token = cursor.CurrentToken();
  if (while_token.kind != lexer::TokenKind::kWhile) {
    Fail(ParserErrorKind::kExpectedWhileKeyword, while_token.range);
  }
  const std::size_t begin_byte_offset = while_token.range.begin_byte_offset;

  cursor.Advance();

  const auto condition_id = ParseParenthesizedExpression(cursor, ast);

  ast::NodeId body_id = ParseBlockStatementImpl(cursor, ast);

  const std::size_t end_byte_offset = ast.GetNode(body_id).range.end_byte_offset;
  const ast::NodeId while_id = ast.AddNode(ast::Node{
    source::SourceRange{begin_byte_offset, end_byte_offset},
    ast::WhileStatement{condition_id, body_id}}
  );

  return while_id;
}

// 現在位置のトークン列がローカル宣言として始まっているか調べる。
bool LooksLikeLocalDeclaration(const TokenCursor& cursor) {
  const lexer::TokenKind current_kind = cursor.CurrentToken().kind;
  if (ToDeclarationModifier(current_kind).has_value()) {
    return true;
  }
  if (current_kind != lexer::TokenKind::kIdentifier) {
    return false;
  }

  const lexer::TokenKind next_kind = cursor.PeekToken(1).kind;
  return next_kind == lexer::TokenKind::kIdentifier
    || ToDeclarationModifier(next_kind).has_value();
}

// 現在位置のトークン列が代入として始まっているか調べる。
bool LooksLikeAssignment(const TokenCursor& cursor) {
  return cursor.CurrentToken().kind == lexer::TokenKind::kIdentifier
    && cursor.PeekToken(1).kind == lexer::TokenKind::kEqual;
}

ast::NodeId ParseStatementImpl(
  TokenCursor& cursor,
  ast::Ast& ast
) {
  const lexer::TokenKind current_kind = cursor.CurrentToken().kind;

  if (current_kind == lexer::TokenKind::kLeftBrace) {
    return ParseBlockStatementImpl(cursor, ast);
  }
  if (current_kind == lexer::TokenKind::kReturn) {
    return ParseReturnStatementImpl(cursor, ast);
  }
  if (current_kind == lexer::TokenKind::kIf) {
    return ParseIfStatement(cursor, ast);
  }
  if (current_kind == lexer::TokenKind::kWhile) {
    return ParseWhileStatement(cursor, ast);
  }
  if (current_kind == lexer::TokenKind::kBreak) {
    return ParseBreakStatement(cursor, ast);
  }
  if (current_kind == lexer::TokenKind::kContinue) {
    return ParseContinueStatement(cursor, ast);
  }
  if (current_kind == lexer::TokenKind::kLeftParenthesis) {
    return ParseCallStatement(cursor, ast);
  }
  if (LooksLikeLocalDeclaration(cursor)) {
    return ParseLocalDeclarationStatementImpl(cursor, ast);
  }
  if (LooksLikeAssignment(cursor)) {
    return ParseAssignmentStatementImpl(cursor, ast);
  }
  if (current_kind == lexer::TokenKind::kIdentifier && cursor.PeekToken(1).kind == lexer::TokenKind::kLeftParenthesis) {
    return ParseCallStatement(cursor, ast);
  }

  Fail(ParserErrorKind::kExpectedStatement, cursor.CurrentToken().range);
}

ast::NodeId ParseBlockStatementImpl(
  TokenCursor& cursor,
  ast::Ast& ast
) {
  throw std::logic_error("not yet implemented: ParseBlockStatementImpl");
}

// 現在位置から関数パラメータを解析する。
ast::NodeId ParseFunctionParameter(
  TokenCursor& cursor,
  ast::Ast& ast
) {
  const std::vector<ast::DeclarationComponent> components = ParseDeclarationComponents(cursor);
  if (components.size() < 2) {
    Fail(
      ParserErrorKind::kExpectedDeclarationComponent,
      cursor.CurrentToken().range
    );
  }

  const lexer::TokenKind current_kind = cursor.CurrentToken().kind;
  if (current_kind != lexer::TokenKind::kComma && current_kind != lexer::TokenKind::kRightParenthesis) {
    Fail(
      ParserErrorKind::kExpectedCommaOrRightParenthesis,
      cursor.CurrentToken().range
    );
  }

  const ast::NodeId parameter_id = ast.AddNode(ast::Node{
    source::SourceRange{components.front().range.begin_byte_offset, components.back().range.end_byte_offset},
    ast::FunctionParameter{components}
  });

  return parameter_id;
}

// トークンを関数ヘッダの構成要素に変換する。
// 関数ヘッダの構成要素でなければstd::nulloptを返す。
std::optional<ast::FunctionHeadComponent> ToFunctionHeadComponent(const lexer::Token& token) {
  if (token.kind == lexer::TokenKind::kIdentifier) {
    return ast::FunctionHeadComponent{
      token.range,
      ast::DeclarationIdentifier{token.normalized_identifier}
    };
  }
  if (token.kind == lexer::TokenKind::kVoid) {
    return ast::FunctionHeadComponent{token.range, ast::VoidReturnMarker{}};
  }
  return std::nullopt;
}

ast::NodeId ParseFunctionDefinitionImpl(
  TokenCursor& cursor,
  ast::Ast& ast
) {
  const std::size_t begin_byte_offset = cursor.CurrentToken().range.begin_byte_offset;
  std::vector<ast::FunctionHeadComponent> head_components;
  // 関数ヘッダは、返り値と関数名の2要素で構成される。
  while (head_components.size() < 2) {
    const std::optional<ast::FunctionHeadComponent> component = ToFunctionHeadComponent(cursor.CurrentToken());
    if (!component.has_value()) {
      Fail(ParserErrorKind::kExpectedFunctionHeadComponent, cursor.CurrentToken().range);
    }
    head_components.push_back(*component);
    cursor.Advance();
  }

  Consume(
    cursor,
    lexer::TokenKind::kLeftParenthesis,
    ParserErrorKind::kExpectedLeftParenthesis
  );

  std::vector<ast::NodeId> parameters;
  if (cursor.CurrentToken().kind != lexer::TokenKind::kRightParenthesis) {
    while (true) {
      ast::NodeId parameter_id = ParseFunctionParameter(cursor, ast);
      parameters.push_back(parameter_id);
      if (cursor.CurrentToken().kind == lexer::TokenKind::kRightParenthesis) {
        break;
      }
      // パラメータ解析成功後に右括弧でなければ、現在位置はコンマ
      cursor.Advance();
    }
  }
  cursor.Advance();

  ast::NodeId body_id = ParseBlockStatementImpl(cursor, ast);

  const ast::Node& body_node = ast.GetNode(body_id);
  const ast::NodeId function_id = ast.AddNode(ast::Node{
    source::SourceRange{begin_byte_offset, body_node.range.end_byte_offset},
    ast::FunctionDefinition{head_components, parameters, body_id}}
  );

  return function_id;
}

ast::NodeId ParseCompilationUnitImpl(
  TokenCursor& cursor,
  ast::Ast& ast
) {
  throw std::logic_error("not yet implemented: ParseCompilationUnitImpl");
}

} // namespace

ParseResult ParseExpression(
  TokenCursor& cursor,
  ast::Ast& ast
) {
  return AtEntry<ParseExpressionImpl>(cursor, ast);
}

ParseResult ParseLocalDeclarationStatement(
  TokenCursor& cursor,
  ast::Ast& ast
) {
  return AtEntry<ParseLocalDeclarationStatementImpl>(cursor, ast);
}

ParseResult ParseAssignmentStatement(
  TokenCursor& cursor,
  ast::Ast& ast
) {
  return AtEntry<ParseAssignmentStatementImpl>(cursor, ast);
}

ParseResult ParseReturnStatement(
  TokenCursor& cursor,
  ast::Ast& ast
) {
  return AtEntry<ParseReturnStatementImpl>(cursor, ast);
}

ParseResult ParseStatement(
  TokenCursor& cursor,
  ast::Ast& ast
) {
  return AtEntry<ParseStatementImpl>(cursor, ast);
}

ParseResult ParseBlockStatement(
  TokenCursor& cursor,
  ast::Ast& ast
) {
  return AtEntry<ParseBlockStatementImpl>(cursor, ast);
}

ParseResult ParseFunctionDefinition(
  TokenCursor& cursor,
  ast::Ast& ast
) {
  return AtEntry<ParseFunctionDefinitionImpl>(cursor, ast);
}

ParseResult ParseCompilationUnit(
  TokenCursor& cursor,
  ast::Ast& ast
) {
  return AtEntry<ParseCompilationUnitImpl>(cursor, ast);
}

} // namespace epp::parser
