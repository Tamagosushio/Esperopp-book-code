#include "epp/sema/name_resolution.h"

#include <stdexcept>
#include <optional>
#include <string>
#include <string_view>
#include <variant>

#include "epp/ast/node.h"
#include "epp/morphology/identifier.h"
#include "epp/sema/function_table.h"
#include "epp/sema/scope.h"

namespace epp::sema {

VariableNameResolutionResult ResolveVariableName(
  ast::NodeId expression_id,
  const ast::Ast& ast,
  const ScopeStack& scope_stack
) {
  const ast::Node& node = ast.GetNode(expression_id);
  const auto* name_expression = std::get_if<ast::NameExpression>(&node.data);
  if (name_expression == nullptr) {
    return Error{ErrorKind::kExpectedNameExpression, node.range};
  }

  const std::optional<VariableSymbolId> variable_id
    = scope_stack.LookupVariable(name_expression->normalized_identifier);
  if (!variable_id.has_value()) {
    return Error{ErrorKind::kUndefinedVariable, node.range};
  }

  return *variable_id;
}

VariableNameResolutionResult ResolveMutableVariableName(
  ast::NodeId expression_id,
  const ast::Ast& ast,
  const ScopeStack& scope_stack
) {
  const auto result = ResolveVariableName(expression_id, ast, scope_stack);
  const auto* variable_id = std::get_if<VariableSymbolId>(&result);
  if (variable_id != nullptr && scope_stack.GetVariable(*variable_id).is_constant) {
    return Error{
      ErrorKind::kConstantAssignmentTarget,
      ast.GetNode(expression_id).range
    };
  }
  return result;
}

std::optional<FunctionSymbolId> ResolveDirectCallFunction(
  std::u32string_view direct_call_identifier,
  const FunctionTable& function_table
) {
  const std::optional<std::u32string> function_identifier
    = morphology::GetFunctionIdentifierForDirectCall(direct_call_identifier);
  if (!function_identifier.has_value()) {
    return std::nullopt;
  }

  return function_table.LookupFunction(*function_identifier);
}

CallTargetResolutionResult ResolveCallTarget(
  ast::NodeId expression_id,
  const ast::Ast& ast,
  const FunctionTable& function_table
) {
  throw std::logic_error("not yet implemented: call-target");
}

} // namespace epp::sema
