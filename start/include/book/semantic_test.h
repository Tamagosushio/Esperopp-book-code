#ifndef BOOK_SEMANTIC_TEST_H_
#define BOOK_SEMANTIC_TEST_H_

#include <iostream>
#include <string_view>
#include <variant>

#include "book/test.h"
#include "epp/ast/node.h"
#include "epp/lexer/lexer.h"
#include "epp/parser/parser.h"
#include "epp/sema/declaration.h"
#include "epp/sema/function_table.h"
#include "epp/sema/name_resolution.h"
#include "epp/sema/program.h"
#include "epp/sema/scope.h"

namespace book {

inline int TestScopes() {
  using namespace epp::sema;
  ScopeStack scopes;
  const VariableSymbol value{U"nombro", BuiltInType::kInteger, false, {}};
  const auto outer = scopes.DeclareVariable(value);
  const auto duplicate = scopes.DeclareVariable(value);
  scopes.EnterScope();
  const auto inner = scopes.DeclareVariable(value);
  const bool found_inner = scopes.LookupVariable(U"nombro") == inner;
  const bool exited = scopes.ExitScope();
  const bool found_outer = scopes.LookupVariable(U"nombro") == outer;
  std::cout << "same scope: " << (duplicate ? "accepted" : "rejected") << '\n';
  std::cout << "inner scope: " << (found_inner ? "inner" : "other") << '\n';
  std::cout << "after exit: " << (found_outer ? "outer" : "other") << '\n';
  return outer && !duplicate && inner && found_inner && exited && found_outer ? 0 : 1;
}

inline int TestDirectCall() {
  using namespace epp;
  sema::FunctionTable functions;
  const auto defined = functions.DeclareFunction(sema::FunctionSymbol{
    U"duobligi", {sema::BuiltInType::kInteger}, sema::BuiltInType::kInteger,{}
  });
  const auto lexed = lexer::LexSource("duobligu(5)", 0);
  if (!defined || lexed.error) return 1;
  parser::TokenCursor cursor(lexed.tokens);
  ast::Ast tree;
  const auto result = parser::ParseExpression(cursor, tree);
  const auto* expression = std::get_if<ast::NodeId>(&result);
  if (!expression || !cursor.IsAtEnd()) return 1;
  const auto found = sema::ResolveCallTarget(*expression, tree, functions);
  const auto* found_id = std::get_if<sema::FunctionSymbolId>(&found);
  if (!found_id || *found_id != *defined) return 1;
  std::cout << "function: " << Utf8(functions.GetFunction(*found_id).normalized_identifier) << '\n';
  return 0;
}

inline const char* SemanticErrorName(epp::sema::ErrorKind kind) {
  using epp::sema::ErrorKind;
  switch (kind) {
    case ErrorKind::kInvalidInitializerType:
      return "invalid initializer type";
    case ErrorKind::kInvalidReturnType:
      return "invalid return type";
    case ErrorKind::kUndefinedVariable:
      return "undefined variable";
    case ErrorKind::kDuplicateVariableName:
      return "duplicate variable";
    default:
      return "semantic error";
  }
}

inline int TestLocalDeclaration(std::string_view input) {
  using namespace epp;
  const auto lexed = lexer::LexSource(input, 0);
  if (lexed.error) return 1;
  parser::TokenCursor cursor(lexed.tokens);
  ast::Ast tree;
  const auto parsed = parser::ParseLocalDeclarationStatement(cursor, tree);
  const auto* id = std::get_if<ast::NodeId>(&parsed);
  if (!id || !cursor.IsAtEnd()) return 1;
  const auto& declaration = std::get<ast::LocalDeclarationStatement>(tree.GetNode(*id).data);
  sema::ScopeStack scopes;
  sema::FunctionTable functions;
  const auto result = sema::RegisterLocalDeclaration(declaration, tree, scopes, functions);
  if (const auto* error = std::get_if<sema::Error>(&result)) {
    std::cerr << "error: " << SemanticErrorName(error->kind) << '\n';
    return 1;
  }
  const auto id_value = std::get<sema::VariableSymbolId>(result);
  const auto& variable = scopes.GetVariable(id_value);
  std::cout << "name: " << Utf8(variable.normalized_identifier) << '\n';
  std::cout << "type: " << (variable.type == sema::BuiltInType::kInteger ? "entjera" : "other") << '\n';
  return 0;
}

inline int TestSemantics(std::string_view input) {
  using namespace epp;
  const auto lexed = lexer::LexSource(input, 0);
  if (lexed.error) return 1;
  parser::TokenCursor cursor(lexed.tokens);
  ast::Ast tree;
  const auto parsed = parser::ParseCompilationUnit(cursor, tree);
  const auto* id = std::get_if<ast::NodeId>(&parsed);
  if (!id) return 1;
  const auto& unit = std::get<ast::CompilationUnit>(tree.GetNode(*id).data);
  const auto result = sema::AnalyzeProgram(unit, tree);
  if (const auto* error = std::get_if<sema::Error>(&result)) {
    std::cerr << "error: " << SemanticErrorName(error->kind) << '\n';
    return 1;
  }
  const auto& program = std::get<sema::AnalyzedProgram>(result);
  const auto& entry = program.function_table.GetFunction(program.entry_point);
  std::cout << "entry: " << Utf8(entry.normalized_identifier) << '\n';
  std::cout << "user functions: " << program.function_ids.size() << '\n';
  return 0;
}

}  // namespace book

#endif  // BOOK_SEMANTIC_TEST_H_
