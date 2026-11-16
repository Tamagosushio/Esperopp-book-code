#include "epp/sema/function.h"

#include <algorithm>
#include <cstddef>
#include <optional>
#include <utility>
#include <variant>
#include <vector>

#include "epp/ast/node.h"
#include "epp/morphology/identifier.h"
#include "epp/sema/declaration.h"
#include "epp/sema/flow.h"
#include "epp/sema/function_table.h"
#include "epp/sema/scope.h"
#include "epp/sema/statement.h"
#include "epp/sema/type.h"
#include "identifier_rules.h"

namespace epp::sema {

namespace {

// 関数パラメータの解析結果を表す。
using FunctionParameterAnalysisResult = std::variant<std::vector<VariableSymbol>, Error>;

FunctionParameterAnalysisResult AnalyzeFunctionParameters(
  const ast::FunctionDefinition& function_definition,
  const ast::Ast& ast
) {
  std::vector<VariableSymbol> variables;

  for (ast::NodeId parameter_id : function_definition.parameters) {
    const ast::Node& parameter_node = ast.GetNode(parameter_id);
    const auto* parameter = std::get_if<ast::FunctionParameter>(&parameter_node.data);
    if (parameter == nullptr) {
      return Error{
        ErrorKind::kExpectedFunctionParameter,
        parameter_node.range
      };
    }

    const DeclarationAnalysisResult declaration_result = AnalyzeDeclarationComponents(parameter->components);
    const auto* analyzed_declaration = std::get_if<AnalyzedDeclaration>(&declaration_result);
    if (analyzed_declaration == nullptr) {
      const Error& error = std::get<Error>(declaration_result);
      return error;
    }

    const auto duplicate_parameter = std::find_if(
      variables.begin(),
      variables.end(),
      [&analyzed_declaration](const VariableSymbol& variable) {
        return variable.normalized_identifier == analyzed_declaration->normalized_identifier;
      }
    );
    if (duplicate_parameter != variables.end()) {
      return Error{
        ErrorKind::kDuplicateParameterName,
        analyzed_declaration->name_range
      };
    }

    variables.push_back(VariableSymbol{
      analyzed_declaration->normalized_identifier,
      analyzed_declaration->type,
      analyzed_declaration->is_constant,
      analyzed_declaration->name_range
    });
  }

  return variables;
}

} // namespace

FunctionHeadAnalysisResult AnalyzeFunctionHeadComponents(
  const std::vector<ast::FunctionHeadComponent>& components
) {
  if (components.size() != 2) {
    return Error{
      ErrorKind::kInvalidFunctionHeadComponents,
      components.empty() ? std::nullopt : std::optional{components.back().range}
    };
  }
  std::optional<std::size_t> type_index;
  std::optional<std::size_t> name_index;
  for (std::size_t i = 0; i < components.size(); ++i) {
    const auto* identifier = std::get_if<ast::DeclarationIdentifier>(&components[i].data);
    if (!identifier) {
      if (type_index) {
        return Error{ErrorKind::kInvalidFunctionHeadComponents, components[i].range};
      }
      type_index = i;
      continue;
    }

    const auto info = morphology::AnalyzeIdentifier(identifier->normalized_identifier);
    if (const auto error = detail::CheckIdentifierEndings(info, components[i].range)) {
      return *error;
    }
    switch (morphology::ClassifyIdentifierRole(info)) {
      case morphology::IdentifierRole::kTypeName:
        if (type_index) {
          return Error{ErrorKind::kInvalidFunctionHeadComponents, components[i].range};
        }
        type_index = i;
        break;
      case morphology::IdentifierRole::kFunctionName:
        if (name_index) {
          return Error{ErrorKind::kInvalidFunctionHeadComponents, components[i].range};
        }
        name_index = i;
        break;
      default:
        return Error{ErrorKind::kFunctionHeadInvalidIdentifierRole, components[i].range};
    }
  }
  std::optional<BuiltInType> type;
  if (const auto* identifier = std::get_if<ast::DeclarationIdentifier>(&components[*type_index].data)) {
    type = ResolveBuiltInTypeName(identifier->normalized_identifier);
    if (!type) {
      return Error{ErrorKind::kUnknownReturnTypeName, components[*type_index].range};
    }
  }

  return AnalyzedFunctionHead{type, *name_index};
}

using FunctionRegistrationResult = std::variant<FunctionSymbolId, Error>;

FunctionRegistrationResult RegisterFunctionDefinition(
  const ast::FunctionDefinition& function_definition,
  const ast::Ast& ast,
  FunctionTable& function_table
) {
  const FunctionHeadAnalysisResult head_result
    = AnalyzeFunctionHeadComponents(function_definition.head_components);
  const auto* function_head = std::get_if<AnalyzedFunctionHead>(&head_result);
  if (function_head == nullptr) {
    return std::get<Error>(head_result);
  }

  const ast::FunctionHeadComponent& function_name_component
    = function_definition.head_components[function_head->name_index];
  const auto* function_name
    = std::get_if<ast::DeclarationIdentifier>(&function_name_component.data);
  const FunctionParameterAnalysisResult parameter_result = AnalyzeFunctionParameters(function_definition, ast);
  const auto* analyzed_parameters = std::get_if<std::vector<VariableSymbol>>(&parameter_result);
  if (analyzed_parameters == nullptr) {
    return std::get<Error>(parameter_result);
  }

  std::vector<BuiltInType> parameter_types;
  parameter_types.reserve(analyzed_parameters->size());
  for (const VariableSymbol& variable : *analyzed_parameters) {
    parameter_types.push_back(variable.type);
  }

  const std::optional<FunctionSymbolId> function_id = function_table.DeclareFunction(FunctionSymbol{
    function_name->normalized_identifier,
    parameter_types,
    function_head->return_type,
    function_name_component.range
  });
  if (!function_id.has_value()) {
    return Error{ErrorKind::kDuplicateFunctionName, function_name_component.range};
  }

  return *function_id;
}

FunctionCollectionResult RegisterCompilationUnitFunctions(
  const ast::CompilationUnit& compilation_unit,
  const ast::Ast& ast,
  FunctionTable& function_table
) {
  FunctionTable updated_function_table = function_table;
  std::vector<FunctionSymbolId> function_ids;
  for (ast::NodeId function_id : compilation_unit.function_definitions) {
    const ast::Node& function_node = ast.GetNode(function_id);
    const auto* function_definition = std::get_if<ast::FunctionDefinition>(&function_node.data);
    if (function_definition == nullptr) {
      return Error{ErrorKind::kExpectedFunctionDefinition, function_node.range};
    }

    const FunctionRegistrationResult registration_result
      = RegisterFunctionDefinition(*function_definition, ast, updated_function_table);
    const auto* registered_function_id = std::get_if<FunctionSymbolId>(&registration_result);
    if (registered_function_id == nullptr) {
      return std::get<Error>(registration_result);
    }

    function_ids.push_back(*registered_function_id);
  }

  // 全関数の登録に成功したら、元の関数表を更新する。
  function_table = std::move(updated_function_table);

  return function_ids;
}

FunctionParameterRegistrationResult RegisterFunctionParameters(
  const ast::FunctionDefinition& function_definition,
  const ast::Ast& ast,
  ScopeStack& scope_stack
) {
  const FunctionParameterAnalysisResult parameter_result = AnalyzeFunctionParameters(function_definition, ast);
  const auto* analyzed_parameters = std::get_if<std::vector<VariableSymbol>>(&parameter_result);
  if (analyzed_parameters == nullptr) {
    return std::get<Error>(parameter_result);
  }

  ScopeStack updated_scope_stack = scope_stack;
  std::vector<VariableSymbolId> variable_ids;
  for (const VariableSymbol& variable : *analyzed_parameters) {
    const std::optional<VariableSymbolId> variable_id = updated_scope_stack.DeclareVariable(variable);
    if (!variable_id.has_value()) {
      return Error{ErrorKind::kDuplicateParameterName, variable.declaration_range};
    }

    variable_ids.push_back(*variable_id);
  }

  scope_stack = std::move(updated_scope_stack);

  return variable_ids;
}

namespace {

CheckResult AnalyzeFunctionBody(
  const ast::FunctionDefinition& function_definition,
  const ast::Ast& ast,
  const FunctionTable& function_table
) {
  const auto head_result = AnalyzeFunctionHeadComponents(function_definition.head_components);
  if (const auto* error = std::get_if<Error>(&head_result)) {
    return *error;
  }
  const auto return_type = std::get<AnalyzedFunctionHead>(head_result).return_type;

  const ast::Node& body_node = ast.GetNode(function_definition.body);
  const auto* body = std::get_if<ast::BlockStatement>(&body_node.data);
  if (body == nullptr) {
    return Error{ErrorKind::kExpectedBlockStatement, body_node.range};
  }

  ScopeStack scope_stack;
  const auto parameter_result = RegisterFunctionParameters(function_definition, ast, scope_stack);
  if (const auto* error = std::get_if<Error>(&parameter_result)) {
    return *error;
  }

  for (const ast::NodeId statement_id : body->statements) {
    if (const auto error = AnalyzeStatement(statement_id, ast, scope_stack, function_table, return_type)) {
       return error;
    }
  }

  const auto flow_result = AnalyzeStatementFlow(function_definition.body, ast);
  if (const auto* error = std::get_if<Error>(&flow_result)) {
    return *error;
  }
  if (return_type && std::get<StatementFlow>(flow_result).can_complete_normally) {
    return Error{ErrorKind::kMissingReturn, body_node.range};
  }

  return std::nullopt;
}

} // namespace

CompilationUnitAnalysisResult AnalyzeCompilationUnit(
  const ast::CompilationUnit& compilation_unit,
  const ast::Ast& ast,
  FunctionTable& function_table,
  std::vector<FunctionSymbolId>* function_ids
) {
  FunctionTable updated_function_table = function_table;
  auto registration_result = RegisterCompilationUnitFunctions(compilation_unit, ast, updated_function_table);
  if (const auto* error = std::get_if<Error>(&registration_result)) {
    return *error;
  }

  for (const ast::NodeId function_id : compilation_unit.function_definitions) {
    const auto& definition = std::get<ast::FunctionDefinition>(ast.GetNode(function_id).data);
    if (const auto error = AnalyzeFunctionBody(definition, ast, updated_function_table)) {
      return error;
    }
  }

  function_table = std::move(updated_function_table);
  if (function_ids) {
    *function_ids = std::get<std::vector<FunctionSymbolId>>(std::move(registration_result));
  }
  return std::nullopt;
}

EntryPointAnalysisResult AnalyzeEntryPoint(const FunctionTable& function_table) {
  const auto function_id = function_table.LookupFunction(U"ĉefi");
  if (!function_id.has_value()) {
    return Error{ErrorKind::kMissingEntryPoint, std::nullopt};
  }
  const FunctionSymbol& function = function_table.GetFunction(*function_id);
  if (!function.parameter_types.empty()) {
    return Error{ErrorKind::kUnexpectedParameters, function.declaration_range};
  }
  if (function.return_type != BuiltInType::kInteger) {
    return Error{ErrorKind::kEntryPointInvalidReturnType, function.declaration_range};
  }
  return *function_id;
}

} // namespace epp::sema
