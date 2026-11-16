#ifndef EPP_INCLUDE_EPP_SEMA_ERROR_H_
#define EPP_INCLUDE_EPP_SEMA_ERROR_H_

#include <optional>

#include "epp/source/source_range.h"

namespace epp::sema {

// 意味解析の各段階で同じエラーをそのまま伝播する。
enum class ErrorKind {
  kMissingReturnValue,
  kInvalidReturnType,
  kInvalidAssignmentTarget,
  kUndefinedVariable,
  kConstantAssignmentTarget,
  kInvalidAssignmentType,
  kExpectedFunctionParameter,
  kDuplicateParameterName,
  kDuplicateFunctionName,
  kExpectedFunctionDefinition,
  kMissingEntryPoint,
  kUnexpectedParameters,
  kEntryPointInvalidReturnType,
  kUnexpectedFlowStatement,
  kMissingReturn,
  kExpectedNameExpression,
  kExpectedExpression,
  kExpectedCallExpression,
  kInvalidUnaryOperand,
  kInvalidBinaryOperands,
  kInvalidBinaryOperator,
  kInvalidOrderingOperands,
  kInvalidCallTarget,
  kInvalidDirectCallName,
  kUndefinedFunction,
  kArgumentCountMismatch,
  kInvalidArgumentType,
  kExpectedValue,
  kIntegerLiteralOutOfRange,
  kUnexpectedStatementNode,
  kInvalidConditionType,
  kBreakOutsideLoop,
  kContinueOutsideLoop,
  kExpectedBlockStatement,
  kAccusativeEndingReserved,
  kPluralEndingForbidden,
  kInvalidIdentifierRole,
  kUnknownTypeName,
  kFunctionHeadInvalidIdentifierRole,
  kUnknownReturnTypeName,
  kDuplicateVariableName,
  kInvalidInitializerType,
  kInvalidDeclarationComponents,
  kInvalidFunctionHeadComponents,
};

// 意味解析のエラーと、元ソースでの検出範囲。
// 開始関数が存在しない場合など、範囲を指定できないエラーではrangeをnulloptにする。
struct Error {
  ErrorKind kind;
  std::optional<source::SourceRange> range;
};

// 値を返さない意味検査。成功はnullopt、失敗は元のエラー。
using CheckResult = std::optional<Error>;

} // namespace epp::sema

#endif // EPP_INCLUDE_EPP_SEMA_ERROR_H_
