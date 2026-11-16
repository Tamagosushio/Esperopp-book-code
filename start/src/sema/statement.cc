#include "epp/sema/statement.h"

#include <optional>
#include <variant>

#include "epp/sema/declaration.h"
#include "epp/sema/expression.h"
#include "epp/sema/name_resolution.h"

namespace epp::sema {

namespace {

StatementAnalysisResult AnalyzeAssignmentStatement(
  const ast::AssignmentStatement& statement,
  const ast::Ast& ast,
  const ScopeStack& scope_stack,
  const FunctionTable& function_table
) {
  const VariableNameResolutionResult target_result
    = ResolveMutableVariableName(statement.target, ast, scope_stack);
  const auto* variable_id = std::get_if<VariableSymbolId>(&target_result);
  if (variable_id == nullptr) {
    const auto& error = std::get<Error>(target_result);
    return Error{
      error.kind == ErrorKind::kExpectedNameExpression
        ? ErrorKind::kInvalidAssignmentTarget
        : error.kind,
      error.range
    };
  }

  const VariableSymbol& variable = scope_stack.GetVariable(*variable_id);

  const ExpressionTypeResult value_result
    = AnalyzeExpression(statement.value, ast, scope_stack, function_table);
  const auto* value_type = std::get_if<BuiltInType>(&value_result);
  if (value_type == nullptr) {
    const auto& error = std::get<Error>(value_result);
    return error;
  }

  if (*value_type != variable.type) {
    return Error{
      ErrorKind::kInvalidAssignmentType,
      ast.GetNode(statement.value).range
    };
  }

  return std::nullopt;
}

StatementAnalysisResult AnalyzeReturnStatement(
  const ast::ReturnStatement& statement,
  source::SourceRange range,
  const ast::Ast& ast,
  const ScopeStack& scope_stack,
  const FunctionTable& function_table,
  std::optional<BuiltInType> return_type
) {
  if (!statement.value) {
    if (return_type) {
      return Error{ErrorKind::kMissingReturnValue, range};
    }
    return std::nullopt;
  }
  const auto value_range = ast.GetNode(*statement.value).range;

  const ExpressionTypeResult value_result
    = AnalyzeExpression(*statement.value, ast, scope_stack, function_table);
  const auto* value_type = std::get_if<BuiltInType>(&value_result);
  if (value_type == nullptr) {
    const auto& error = std::get<Error>(value_result);
    return error;
  }

  if (!return_type || *value_type != *return_type) {
    return Error{ErrorKind::kInvalidReturnType, value_range};
  }

  return std::nullopt;
}

// 成功型、エラー型の順に並ぶ解析結果を、文の解析結果に変換する。
template <typename Success>
[[nodiscard]] StatementAnalysisResult ToStatementResult(
  const std::variant<Success, Error>& result
) {
  if (const auto* error = std::get_if<Error>(&result)) {
    return *error;
  }
  return std::nullopt;
}

// 制御文の条件が、bulea型の値を生成するか検査する。
[[nodiscard]] StatementAnalysisResult AnalyzeCondition(
  ast::NodeId condition_id,
  const ast::Ast& ast,
  const ScopeStack& scope_stack,
  const FunctionTable& function_table
) {
  const auto result = AnalyzeExpression(condition_id, ast, scope_stack, function_table);
  if (const auto* error = std::get_if<Error>(&result)) {
    return *error;
  }
  if (std::get<BuiltInType>(result) != BuiltInType::kBoolean) {
    return Error{
      ErrorKind::kInvalidConditionType,
      ast.GetNode(condition_id).range
    };
  }
  return std::nullopt;
}

StatementAnalysisResult AnalyzeStatementImpl(
  ast::NodeId statement_id,
  const ast::Ast& ast,
  ScopeStack& scope_stack,
  const FunctionTable& function_table,
  std::optional<BuiltInType> return_type,
  bool in_loop
) {
  const ast::Node& node = ast.GetNode(statement_id);
  if (const auto* block = std::get_if<ast::BlockStatement>(&node.data)) {
    ScopeGuard scope{scope_stack};
    for (const ast::NodeId child_id : block->statements) {
      if (const auto error = AnalyzeStatementImpl(child_id, ast, scope_stack, function_table, return_type, in_loop)) {
        return error;
      }
    }
    return std::nullopt;
  }

  if (const auto* if_statement = std::get_if<ast::IfStatement>(&node.data)) {
    for (const ast::IfBranch& branch : if_statement->branches) {
      if (const auto error = AnalyzeCondition(branch.condition, ast, scope_stack, function_table)) {
        return error;
      }
      if (const auto error = AnalyzeStatementImpl(branch.body, ast, scope_stack, function_table, return_type, in_loop)) {
        return error;
      }
    }
    if (if_statement->else_body.has_value()) {
      return AnalyzeStatementImpl(*if_statement->else_body, ast, scope_stack, function_table, return_type, in_loop);
    }
    return std::nullopt;
  }

  if (const auto* while_statement = std::get_if<ast::WhileStatement>(&node.data)) {
    if (const auto error = AnalyzeCondition(while_statement->condition, ast, scope_stack, function_table)) {
      return error;
    }
    return AnalyzeStatementImpl(while_statement->body, ast, scope_stack, function_table, return_type, true);
  }

  if (std::holds_alternative<ast::BreakStatement>(node.data)) {
    if (!in_loop) {
      return Error{ErrorKind::kBreakOutsideLoop, node.range};
    }
    return std::nullopt;
  }

  if (std::holds_alternative<ast::ContinueStatement>(node.data)) {
    if (!in_loop) {
      return Error{ErrorKind::kContinueOutsideLoop, node.range};
    }
    return std::nullopt;
  }

  if (const auto* declaration = std::get_if<ast::LocalDeclarationStatement>(&node.data)) {
    return ToStatementResult(
      RegisterLocalDeclaration(*declaration, ast, scope_stack, function_table)
    );
  }

  if (const auto* statement = std::get_if<ast::AssignmentStatement>(&node.data)) {
    return AnalyzeAssignmentStatement(
      *statement, ast, scope_stack, function_table
    );
  }

  if (const auto* statement = std::get_if<ast::ReturnStatement>(&node.data)) {
    return AnalyzeReturnStatement(
      *statement, node.range, ast, scope_stack, function_table, return_type
    );
  }

  if (std::holds_alternative<ast::CallExpression>(node.data)) {
    return ToStatementResult(
      AnalyzeCallExpression(statement_id, ast, scope_stack, function_table)
    );
  }

  return Error{
    ErrorKind::kUnexpectedStatementNode,
    node.range
  };
}

} // namespace

StatementAnalysisResult AnalyzeStatement(
  ast::NodeId statement_id,
  const ast::Ast& ast,
  ScopeStack& scope_stack,
  const FunctionTable& function_table,
  std::optional<BuiltInType> return_type
) {
  return AnalyzeStatementImpl(statement_id, ast, scope_stack, function_table, return_type, false);
}

} // namespace epp::sema
