#include "epp/sema/type.h"

#include <optional>
#include <string_view>

namespace epp::sema {

namespace {

struct BuiltInTypeName {
  std::u32string_view name;
  BuiltInType type;
};

constexpr BuiltInTypeName kBuiltInTypeNames[]{
  BuiltInTypeName{U"entjera", BuiltInType::kInteger},
  BuiltInTypeName{U"bulea", BuiltInType::kBoolean},
  BuiltInTypeName{U"signa", BuiltInType::kCharacter},
  BuiltInTypeName{U"ĉena", BuiltInType::kString}
};

} // namespace

std::optional<BuiltInType> ResolveBuiltInTypeName(
  std::u32string_view type_name
) noexcept {
  for (const BuiltInTypeName& built_in_type_name : kBuiltInTypeNames) {
    if (type_name == built_in_type_name.name) {
      return built_in_type_name.type;
    }
  }

  return std::nullopt;
}

} // namespace epp::sema
