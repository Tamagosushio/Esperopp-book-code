#include "epp/sema/scope.h"

#include <stdexcept>
#include <cstddef>
#include <optional>
#include <string_view>
#include <utility>

namespace epp::sema {

ScopeStack::ScopeStack() {
  scopes_.push_back(Scope{});
}

void ScopeStack::EnterScope() {
  scopes_.push_back(Scope{});
}

bool ScopeStack::ExitScope() {
  if (scopes_.size() == 1) {
    return false;
  }
  scopes_.pop_back();
  return true;
}

std::optional<VariableSymbolId> ScopeStack::DeclareVariable(
  VariableSymbol symbol
) {
  throw std::logic_error("not yet implemented: declare");
}

std::optional<VariableSymbolId> ScopeStack::LookupVariable(
  std::u32string_view normalized_identifier
) const {
  throw std::logic_error("not yet implemented: lookup");
}

const VariableSymbol& ScopeStack::GetVariable(
  VariableSymbolId symbol_id
) const {
  return variables_.at(symbol_id.index);
}

} // namespace epp::sema
