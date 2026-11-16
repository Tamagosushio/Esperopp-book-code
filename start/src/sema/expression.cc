#include "epp/sema/expression.h"

#include <cstddef>
#include <optional>
#include <variant>

#include "epp/sema/integer_literal.h"
#include "epp/sema/name_resolution.h"

namespace epp::sema {

namespace {

ExpressionTypeResult AnalyzeIntegerLiteral(
  const ast::IntegerLiteralExpression& literal,
  source::SourceRange range,
  bool negate
) {
  if (!FormIntegerLiteralValue(literal.digits, negate).has_value()) {
    return Error{
      ErrorKind::kIntegerLiteralOutOfRange, range
    };
  }
  return BuiltInType::kInteger;
}

ExpressionTypeResult AnalyzeUnaryOperator(
  ast::UnaryOperator operator_kind,
  BuiltInType operand_type,
  source::SourceRange operand_range
) {
  switch (operator_kind) {
    case ast::UnaryOperator::kPlus:
    case ast::UnaryOperator::kMinus:
    case ast::UnaryOperator::kBitwiseNot:
      if (operand_type == BuiltInType::kInteger) {
        return operand_type;
      }
      break;
    case ast::UnaryOperator::kLogicalNot:
      if (operand_type == BuiltInType::kBoolean) {
        return BuiltInType::kBoolean;
      }
      break;
  }

  return Error{
    ErrorKind::kInvalidUnaryOperand,
    operand_range
  };
}

ExpressionTypeResult AnalyzeBinaryOperator(
  ast::BinaryOperator operator_kind,
  BuiltInType left_type,
  BuiltInType right_type,
  source::SourceRange expression_range
) {
  switch (operator_kind) {
    case ast::BinaryOperator::kAdd:
    case ast::BinaryOperator::kSubtract:
    case ast::BinaryOperator::kMultiply:
    case ast::BinaryOperator::kDivide:
    case ast::BinaryOperator::kRemainder:
    case ast::BinaryOperator::kBitwiseOr:
    case ast::BinaryOperator::kBitwiseXor:
    case ast::BinaryOperator::kBitwiseAnd:
    case ast::BinaryOperator::kShiftLeft:
    case ast::BinaryOperator::kShiftRight: {
      if (left_type == BuiltInType::kInteger && right_type == BuiltInType::kInteger) {
        return BuiltInType::kInteger;
      }
      return Error{
        ErrorKind::kInvalidBinaryOperands,
        expression_range
      };
    }

    case ast::BinaryOperator::kLess:
    case ast::BinaryOperator::kLessEqual:
    case ast::BinaryOperator::kGreater:
    case ast::BinaryOperator::kGreaterEqual: {
      if (left_type == BuiltInType::kInteger && right_type == BuiltInType::kInteger) {
        return BuiltInType::kBoolean;
      }
      if (left_type == right_type
        && (left_type == BuiltInType::kCharacter || left_type == BuiltInType::kString)) {
        return Error{
          ErrorKind::kInvalidOrderingOperands,
          expression_range
        };
      }
      break;
    }

    case ast::BinaryOperator::kEqual:
    case ast::BinaryOperator::kNotEqual: {
      if (left_type == right_type) {
        return BuiltInType::kBoolean;
      }
      break;
    }

    case ast::BinaryOperator::kLogicalAnd:
    case ast::BinaryOperator::kLogicalOr: {
      if (left_type == BuiltInType::kBoolean && right_type == BuiltInType::kBoolean) {
        return BuiltInType::kBoolean;
      }
      break;
    }

    default:
      return Error{
        ErrorKind::kInvalidBinaryOperator,
        expression_range
      };
  }

  return Error{
    ErrorKind::kInvalidBinaryOperands,
    expression_range
  };
}

} // namespace

CallAnalysisResult AnalyzeCallExpression(
  ast::NodeId expression_id,
  const ast::Ast& ast,
  const ScopeStack& scope_stack,
  const FunctionTable& function_table
) {
  const CallTargetResolutionResult target_result = ResolveCallTarget(expression_id, ast, function_table);
  const auto* function_id = std::get_if<FunctionSymbolId>(&target_result);
  if (function_id == nullptr) {
    return std::get<Error>(target_result);
  }

  const FunctionSymbol& function = function_table.GetFunction(*function_id);
  const ast::Node& node = ast.GetNode(expression_id);
  const auto& call = std::get<ast::CallExpression>(node.data);
  const bool output = function.builtin == BuiltInFunction::kOutput;
  const auto parameter_count = output ? std::size_t{1} : function.parameter_types.size();
  if (call.arguments.size() != parameter_count) {
    return Error{ErrorKind::kArgumentCountMismatch, node.range};
  }

  for (std::size_t i = 0; i < call.arguments.size(); ++i) {
    const ast::NodeId argument_id = call.arguments[i];
    const ExpressionTypeResult argument_result = AnalyzeExpression(argument_id, ast, scope_stack, function_table);
    const auto* argument_type = std::get_if<BuiltInType>(&argument_result);
    if (argument_type == nullptr) {
      return std::get<Error>(argument_result);
    }
    if (!output && *argument_type != function.parameter_types[i]) {
      return Error{ErrorKind::kInvalidArgumentType, ast.GetNode(argument_id).range};
    }
  }

  return *function_id;
}

ExpressionTypeResult AnalyzeExpression(
  ast::NodeId expression_id,
  const ast::Ast& ast,
  const ScopeStack& scope_stack,
  const FunctionTable& function_table
) {
  const ast::Node& node = ast.GetNode(expression_id);

  if (const auto* literal = std::get_if<ast::IntegerLiteralExpression>(&node.data)) {
    return AnalyzeIntegerLiteral(*literal, node.range, false);
  }
  if (std::holds_alternative<ast::BooleanLiteralExpression>(node.data)) {
    return BuiltInType::kBoolean;
  }
  if (std::holds_alternative<ast::CharacterLiteralExpression>(node.data)) {
    return BuiltInType::kCharacter;
  }
  if (std::holds_alternative<ast::StringLiteralExpression>(node.data)) {
    return BuiltInType::kString;
  }

  if (std::holds_alternative<ast::NameExpression>(node.data)) {
    const auto resolved = ResolveVariableName(expression_id, ast, scope_stack);
    if (const auto* error = std::get_if<Error>(&resolved)) {
      return *error;
    }
    return scope_stack.GetVariable(std::get<VariableSymbolId>(resolved)).type;
  }

  if (const auto* unary = std::get_if<ast::UnaryExpression>(&node.data)) {
    const ast::Node& operand_node = ast.GetNode(unary->operand);
    if (unary->operator_kind == ast::UnaryOperator::kMinus) {
      if (const auto* literal = std::get_if<ast::IntegerLiteralExpression>(&operand_node.data)) {
        return AnalyzeIntegerLiteral(*literal, operand_node.range, true);
      }
    }
    const ExpressionTypeResult operand_result
      = AnalyzeExpression(unary->operand, ast, scope_stack, function_table);
    const auto* operand_type = std::get_if<BuiltInType>(&operand_result);
    if (operand_type == nullptr) {
      return operand_result;
    }
    return AnalyzeUnaryOperator(unary->operator_kind, *operand_type, operand_node.range);
  }

  if (const auto* binary = std::get_if<ast::BinaryExpression>(&node.data)) {
    const ExpressionTypeResult left_result = AnalyzeExpression(binary->left_operand, ast, scope_stack, function_table);
    const auto* left_type = std::get_if<BuiltInType>(&left_result);
    if (left_type == nullptr) {
      return left_result;
    }
    const ExpressionTypeResult right_result = AnalyzeExpression(binary->right_operand, ast, scope_stack, function_table);
    const auto* right_type = std::get_if<BuiltInType>(&right_result);
    if (right_type == nullptr) {
      return right_result;
    }
    return AnalyzeBinaryOperator(binary->operator_kind, *left_type, *right_type, node.range);
  }

  if (std::holds_alternative<ast::CallExpression>(node.data)) {
    const CallAnalysisResult call_result = AnalyzeCallExpression(expression_id, ast, scope_stack, function_table);
    const auto* function_id = std::get_if<FunctionSymbolId>(&call_result);
    if (function_id == nullptr) {
      return std::get<Error>(call_result);
    }
    const std::optional<BuiltInType>& return_type = function_table.GetFunction(*function_id).return_type;
    if (!return_type.has_value()) {
      return Error{ErrorKind::kExpectedValue, node.range};
    }
    return *return_type;
  }

  return Error{ErrorKind::kExpectedExpression, node.range};
}

} // namespace epp::sema
