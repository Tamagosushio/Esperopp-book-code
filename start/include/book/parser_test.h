#ifndef BOOK_PARSER_TEST_H_
#define BOOK_PARSER_TEST_H_

#include <iostream>
#include <string_view>
#include <variant>

#include "book/test.h"
#include "epp/ast/node.h"
#include "epp/lexer/lexer.h"
#include "epp/parser/parser.h"

namespace book {

inline void PrintExpressionTree(const epp::ast::Ast& ast, epp::ast::NodeId id) {
  const auto& data = ast.GetNode(id).data;
  if (const auto* integer = std::get_if<epp::ast::IntegerLiteralExpression>(&data)) {
    std::cout << integer->digits;
    return;
  }
  if (const auto* name = std::get_if<epp::ast::NameExpression>(&data)) {
    std::cout << Utf8(name->normalized_identifier);
    return;
  }
  if (const auto* binary = std::get_if<epp::ast::BinaryExpression>(&data)) {
    std::string_view symbol = "?";
    if (binary->operator_kind == epp::ast::BinaryOperator::kAdd)      symbol = "+";
    if (binary->operator_kind == epp::ast::BinaryOperator::kSubtract) symbol = "-";
    if (binary->operator_kind == epp::ast::BinaryOperator::kMultiply) symbol = "*";
    std::cout << '(' << symbol << ' ';
    PrintExpressionTree(ast, binary->left_operand);
    std::cout << ' ';
    PrintExpressionTree(ast, binary->right_operand);
    std::cout << ')';
    return;
  }
  std::cout << "<other-expression>";
}

inline int TestParse(
  std::string_view input,
  epp::parser::ParseResult (*parse)(epp::parser::TokenCursor&, epp::ast::Ast&),
  std::string_view expected_kind
) {
  const auto lexed = epp::lexer::LexSource(input, 0);
  if (lexed.error) {
    std::cerr << "lex error\n";
    return 1;
  }
  epp::parser::TokenCursor cursor(lexed.tokens);
  epp::ast::Ast ast;
  const auto result = parse(cursor, ast);
  if (const auto* error = std::get_if<epp::parser::ParserError>(&result)) {
    if (error->kind == epp::parser::ParserErrorKind::kExpectedRightBrace) {
      std::cerr << "error: expected }\n";
    } else if (error->kind == epp::parser::ParserErrorKind::kExpectedExpression) {
      std::cerr << "error: expected expression\n";
    } else {
      std::cerr << "parse error\n";
    }
    return 1;
  }
  if (!cursor.IsAtEnd()) {
    std::cerr << "unconsumed tokens\n";
    return 1;
  }
  const auto id = std::get<epp::ast::NodeId>(result);
  const auto& node = ast.GetNode(id);
  if (expected_kind == "expression") {
    PrintExpressionTree(ast, id);
    std::cout << '\n';
    return 0;
  }
  if (expected_kind == "declaration") {
    const auto* decl = std::get_if<epp::ast::LocalDeclarationStatement>(&node.data);
    if (!decl) return 1;
    std::cout << "components:";
    for (const auto& component : decl->components) {
      if (const auto* name = std::get_if<epp::ast::DeclarationIdentifier>(&component.data)) {
        std::cout << ' ' << Utf8(name->normalized_identifier);
      } else {
        std::cout << " konstanta";
      }
    }
    std::cout << "\ninitializer: ";
    if (decl->initializer) {
      PrintExpressionTree(ast, *decl->initializer);
    } else {
      std::cout << "<none>";
    }
    std::cout << '\n';
    return 0;
  }
  if (expected_kind == "block") {
    const auto* block = std::get_if<epp::ast::BlockStatement>(&node.data);
    if (!block) return 1;
    std::cout << "statements: " << block->statements.size() << '\n';
    return 0;
  }
  if (expected_kind == "program") {
    const auto* unit = std::get_if<epp::ast::CompilationUnit>(&node.data);
    if (!unit) return 1;
    std::cout << "functions: " << unit->function_definitions.size() << '\n';
    if (!unit->function_definitions.empty()) {
      const auto& function = std::get<epp::ast::FunctionDefinition>(ast.GetNode(unit->function_definitions.front()).data);
      const auto& body = std::get<epp::ast::BlockStatement>(ast.GetNode(function.body).data);
      std::cout << "first function statements: " << body.statements.size() << '\n';
    }
    return 0;
  }
  return 1;
}

inline int TestExpression(std::string_view input) {
  return TestParse(input, epp::parser::ParseExpression, "expression");
}
inline int TestDeclaration(std::string_view input) {
  return TestParse(input, epp::parser::ParseLocalDeclarationStatement, "declaration");
}
inline int TestBlock(std::string_view input) {
  return TestParse(input, epp::parser::ParseBlockStatement, "block");
}
inline int TestProgram(std::string_view input) {
  return TestParse(input, epp::parser::ParseCompilationUnit, "program");
}

}  // namespace book

#endif  // BOOK_PARSER_TEST_H_
