#include "epp/sema/declaration.h"

#include <stdexcept>
#include <optional>
#include "epp/morphology/identifier.h"
#include "epp/sema/expression.h"
#include "identifier_rules.h"

namespace epp::sema {

DeclarationAnalysisResult AnalyzeDeclarationComponents(
  const std::vector<ast::DeclarationComponent>& components
) {
  const ast::DeclarationComponent* type_component = nullptr;
  const ast::DeclarationComponent* name_component = nullptr;
  bool constant = false;
  for (const auto& component : components) {
    if (std::holds_alternative<ast::DeclarationModifier>(component.data)) {
      if (constant) {
        return Error{
          ErrorKind::kInvalidDeclarationComponents,
          component.range
        };
      }
      constant = true;
      continue;
    }
    const auto& identifier = std::get<ast::DeclarationIdentifier>(component.data);
    const auto info = morphology::AnalyzeIdentifier(identifier.normalized_identifier);
    if (const auto error = detail::CheckIdentifierEndings(info, component.range)) {
      return *error;
    }
    switch (morphology::ClassifyIdentifierRole(info)) {
      case morphology::IdentifierRole::kTypeName:
        if (type_component) {
          return Error{
            ErrorKind::kInvalidDeclarationComponents,
            component.range
          };
        }
        type_component = &component;
        break;
      case morphology::IdentifierRole::kValueName:
        if (name_component) {
          return Error{
            ErrorKind::kInvalidDeclarationComponents,
            component.range
          };
        }
        name_component = &component;
        break;
      default:
        return Error{
          ErrorKind::kInvalidIdentifierRole,
          component.range
        };
    }
  }

  if (!type_component || !name_component) {
    return Error{
      ErrorKind::kInvalidDeclarationComponents,
      components.empty()
        ? std::nullopt
        : std::optional{components.back().range}
    };
  }

  const auto type = ResolveBuiltInTypeName(
    std::get<ast::DeclarationIdentifier>(type_component->data).normalized_identifier
  );
  if (!type) {
    return Error{ErrorKind::kUnknownTypeName, type_component->range};
  }

  return AnalyzedDeclaration{
    std::get<ast::DeclarationIdentifier>(name_component->data).normalized_identifier,
    name_component->range,
    *type,
    constant
  };
}

LocalDeclarationRegistrationResult RegisterLocalDeclaration(
  const ast::LocalDeclarationStatement& declaration,
  const ast::Ast& ast,
  ScopeStack& scope_stack,
  const FunctionTable& function_table
) {
  throw std::logic_error("not yet implemented: local-declaration");
}

} // namespace epp::sema
