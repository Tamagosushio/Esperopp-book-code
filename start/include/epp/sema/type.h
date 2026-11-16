#ifndef EPP_INCLUDE_EPP_SEMA_TYPE_H_
#define EPP_INCLUDE_EPP_SEMA_TYPE_H_

#include <optional>
#include <string_view>

#include "epp/value_type.h"

namespace epp::sema {

// 組み込みの値型を表す。
using BuiltInType = ValueType;

// ソース上の型名を、組込みの型に変換する。
// 対応がなければstd::nulloptを返す。
[[nodiscard]] std::optional<BuiltInType> ResolveBuiltInTypeName(
  std::u32string_view type_name
) noexcept;

} // namespace epp::sema

#endif // EPP_INCLUDE_EPP_SEMA_TYPE_H_
