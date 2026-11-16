#include "epp/diagnostics/sema.h"

#include <stdexcept>

namespace epp::diagnostics {

Diagnostic ToDiagnostic(const sema::Error& error) {
  using sema::ErrorKind;
  const auto make = [&](const char* message) {
    return Diagnostic{
      DiagnosticSeverity::kError,
      message,
      error.range
    };
  };

  switch (error.kind) {
    case ErrorKind::kMissingReturnValue:
      return make("return value is required");
    case ErrorKind::kInvalidReturnType:
      return make("value type must match function return type");
    case ErrorKind::kInvalidAssignmentTarget:
      return make("assignment target must be a variable name");
    case ErrorKind::kUndefinedVariable:
      return make("undefined variable");
    case ErrorKind::kConstantAssignmentTarget:
      return make("cannot assign to a constant");
    case ErrorKind::kInvalidAssignmentType:
      return make("value type must match assignment target type");
    case ErrorKind::kExpectedFunctionParameter:
      throw std::logic_error{"expected a function parameter node in the AST"};
    case ErrorKind::kDuplicateParameterName:
      return make("duplicate parameter name");
    case ErrorKind::kDuplicateFunctionName:
      return make("duplicate function name");
    case ErrorKind::kExpectedFunctionDefinition:
      throw std::logic_error{"expected a function definition node in the AST"};
    case ErrorKind::kMissingEntryPoint:
      return make("missing entry point 'ĉefi'");
    case ErrorKind::kUnexpectedParameters:
      return make("entry point 'ĉefi' must not have parameters");
    case ErrorKind::kEntryPointInvalidReturnType:
      return make("entry point 'ĉefi' must return 'entjera'");
    case ErrorKind::kUnexpectedFlowStatement:
      throw std::logic_error{"unexpected statement node in flow analysis"};
    case ErrorKind::kMissingReturn:
      return make("function may reach the end without returning a value");
    case ErrorKind::kExpectedNameExpression:
      throw std::logic_error{"expected a name expression node in the AST"};
    case ErrorKind::kExpectedExpression:
      throw std::logic_error{"expected an expression node in the AST"};
    case ErrorKind::kExpectedCallExpression:
      throw std::logic_error{"expected a call expression node in the AST"};
    case ErrorKind::kInvalidUnaryOperand:
      return make("invalid operand type for unary operator");
    case ErrorKind::kInvalidBinaryOperands:
      return make("invalid operand types for binary operator");
    case ErrorKind::kInvalidBinaryOperator:
      return make("invalid binary operator");
    case ErrorKind::kInvalidOrderingOperands:
      return make("ordering comparison requires 'entjera' operands");
    case ErrorKind::kInvalidCallTarget:
      return make("call target must be a function name");
    case ErrorKind::kInvalidDirectCallName:
      return make("direct call requires an '-u' name");
    case ErrorKind::kUndefinedFunction:
      return make("undefined function");
    case ErrorKind::kArgumentCountMismatch:
      return make("argument count does not match parameter count");
    case ErrorKind::kInvalidArgumentType:
      return make("argument type does not match parameter type");
    case ErrorKind::kExpectedValue:
      return make("void function call cannot be used as a value");
    case ErrorKind::kIntegerLiteralOutOfRange:
      return make("integer literal is out of range for its type");
    case ErrorKind::kUnexpectedStatementNode:
      throw std::logic_error{"unexpected statement node in the AST"};
    case ErrorKind::kInvalidConditionType:
      return make("condition must have type 'bulea'");
    case ErrorKind::kBreakOutsideLoop:
      return make("break statement is only allowed inside a loop");
    case ErrorKind::kContinueOutsideLoop:
      return make("continue statement is only allowed inside a loop");
    case ErrorKind::kExpectedBlockStatement:
      throw std::logic_error{"expected a block statement node in the AST"};
    case ErrorKind::kAccusativeEndingReserved:
      return make("accusative ending '-n' is reserved");
    case ErrorKind::kPluralEndingForbidden:
      return make("plural ending '-j' is forbidden");
    case ErrorKind::kInvalidIdentifierRole:
      return make("invalid identifier role in declaration");
    case ErrorKind::kUnknownTypeName:
      return make("unknown type name");
    case ErrorKind::kFunctionHeadInvalidIdentifierRole:
      return make("invalid identifier role in function header");
    case ErrorKind::kUnknownReturnTypeName:
      return make("unknown return type name");
    case ErrorKind::kDuplicateVariableName:
      return make("duplicate variable name");
    case ErrorKind::kInvalidInitializerType:
      return make("initializer type must match variable type");
    case ErrorKind::kInvalidDeclarationComponents:
      return make("declaration requires one type, one name and at most one konstanta");
    case ErrorKind::kInvalidFunctionHeadComponents:
      return make("function header requires one return type and one function name");
  }
  throw std::logic_error{"unknown semantic error kind"};
}

} // namespace epp::diagnostics
