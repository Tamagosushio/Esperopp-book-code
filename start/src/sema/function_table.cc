#include "epp/sema/function_table.h"

#include <optional>
#include <string_view>
#include <utility>

namespace epp::sema {

std::optional<FunctionSymbolId> FunctionTable::DeclareFunction(
  FunctionSymbol symbol
) {
  if (LookupFunction(symbol.normalized_identifier).has_value()) {
    return std::nullopt;
  }
  const FunctionSymbolId symbol_id{functions_.size()};
  functions_.push_back(std::move(symbol));

  return symbol_id;
}

std::optional<FunctionSymbolId> FunctionTable::LookupFunction(
  std::u32string_view normalized_identifier
) const {
  for (std::size_t index = 0; index < functions_.size(); ++index) {
    if (functions_[index].normalized_identifier == normalized_identifier) {
      return FunctionSymbolId{index};
    }
  }

  return std::nullopt;
}

const FunctionSymbol& FunctionTable::GetFunction(
  FunctionSymbolId symbol_id
) const {
  return functions_.at(symbol_id.index);
}

} // namespace epp::sema
