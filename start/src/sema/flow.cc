#include "epp/sema/flow.h"

namespace epp::sema {

namespace {

void MergeAlternativeFlow(
  StatementFlow& flow,
  const StatementFlow& alternative
) {
  flow.can_complete_normally |= alternative.can_complete_normally;
  flow.can_break |= alternative.can_break;
}

} // namespace

FlowAnalysisResult AnalyzeStatementFlow(
  ast::NodeId statement_id,
  const ast::Ast& ast
) {
  const ast::Node& node = ast.GetNode(statement_id);
  if (const auto* block = std::get_if<ast::BlockStatement>(&node.data)) {
    StatementFlow flow{.can_complete_normally = true};
    for (const ast::NodeId child_id : block->statements) {
      if (!flow.can_complete_normally) {
        break;
      }
      const auto result = AnalyzeStatementFlow(child_id, ast);
      if (const auto* error = std::get_if<Error>(&result)) {
        return *error;
      }
      const auto& child_flow = std::get<StatementFlow>(result);
      flow.can_complete_normally = child_flow.can_complete_normally;
      flow.can_break |= child_flow.can_break;
    }
    return flow;
  }

  if (const auto* if_statement = std::get_if<ast::IfStatement>(&node.data)) {
    StatementFlow flow;
    bool can_reach_next_branch = true;
    for (const ast::IfBranch& branch : if_statement->branches) {
      if (!can_reach_next_branch) {
        break;
      }
      const auto* literal = std::get_if<ast::BooleanLiteralExpression>(&ast.GetNode(branch.condition).data);
      if (literal != nullptr && !literal->value) {
        continue;
      }
      const auto result = AnalyzeStatementFlow(branch.body, ast);
      if (const auto* error = std::get_if<Error>(&result)) {
        return *error;
      }
      MergeAlternativeFlow(flow, std::get<StatementFlow>(result));
      can_reach_next_branch = literal == nullptr;
    }
    if (can_reach_next_branch) {
      if (if_statement->else_body.has_value()) {
        const auto result = AnalyzeStatementFlow(*if_statement->else_body, ast);
        if (const auto* error = std::get_if<Error>(&result)) {
          return *error;
        }
        MergeAlternativeFlow(flow, std::get<StatementFlow>(result));
      } else {
        flow.can_complete_normally = true;
      }
    }
    return flow;
  }

  if (const auto* while_statement = std::get_if<ast::WhileStatement>(&node.data)) {
    const auto* literal = std::get_if<ast::BooleanLiteralExpression>(
      &ast.GetNode(while_statement->condition).data
    );
    if (literal != nullptr && !literal->value) {
      return StatementFlow{.can_complete_normally = true};
    }
    const auto result = AnalyzeStatementFlow(while_statement->body, ast);
    if (const auto* error = std::get_if<Error>(&result)) {
      return *error;
    }
    const auto& body_flow = std::get<StatementFlow>(result);
    return StatementFlow{
      .can_complete_normally = literal == nullptr || body_flow.can_break
    };
  }

  if (std::holds_alternative<ast::ReturnStatement>(node.data)) {
    return StatementFlow{};
  }
  if (std::holds_alternative<ast::BreakStatement>(node.data)) {
    return StatementFlow{.can_break = true};
  }
  if (std::holds_alternative<ast::ContinueStatement>(node.data)) {
    return StatementFlow{};
  }
  if (std::holds_alternative<ast::LocalDeclarationStatement>(node.data)
    || std::holds_alternative<ast::AssignmentStatement>(node.data)
    || std::holds_alternative<ast::CallExpression>(node.data)) {
    return StatementFlow{.can_complete_normally = true};
  }
  return Error{ErrorKind::kUnexpectedFlowStatement, node.range};
}

} // namespace epp::sema
