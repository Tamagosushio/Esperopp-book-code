#ifndef EPP_INCLUDE_EPP_SEMA_FUNCTION_TABLE_H_
#define EPP_INCLUDE_EPP_SEMA_FUNCTION_TABLE_H_

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "epp/sema/type.h"
#include "epp/source/source_range.h"

namespace epp::sema {

// 関数シンボルを識別する番号を表す。
// 所属するFunctionTable内で有効。ASTのノード番号やIRの関数番号とは区別する。
struct FunctionSymbolId {
  std::size_t index;
  [[nodiscard]] bool operator==(const FunctionSymbolId& other) const noexcept = default;
};

// 利用者定義の関数と、入出力・整数テープの組み込み関数を区別する。
enum class BuiltInFunction {
  kNone,
  kOutput,
  kInput,
  kStringLength,
  kStringCharacter,
  kTapeStep
};

// 宣言された関数について、意味解析で必要になる情報を保持する。
// 値を返さない関数では、return_typeがstd::nulloptになる。
struct FunctionSymbol {
  std::u32string normalized_identifier;
  std::vector<BuiltInType> parameter_types;
  std::optional<BuiltInType> return_type;
  source::SourceRange declaration_range;
  BuiltInFunction builtin = BuiltInFunction::kNone;
};

// ファイル内で宣言された関数を管理する。
class FunctionTable {
 public:
  // 関数を登録する。
  // 同名の関数が登録済みなら、std::nulloptを返す。
  [[nodiscard]] std::optional<FunctionSymbolId> DeclareFunction(
    FunctionSymbol symbol
  );
  // 不定詞形の関数名と一致する関数を探す。
  [[nodiscard]] std::optional<FunctionSymbolId> LookupFunction(
    std::u32string_view normalized_identifier
  ) const;
  // IDに対応する関数情報を返す。
  // 不正なIDではout_of_rangeを送出する。
  // 関数の追加により以前の参照が無効になる可能性がある。
  [[nodiscard]] const FunctionSymbol& GetFunction(
    FunctionSymbolId symbol_id
  ) const;

 private:
  std::vector<FunctionSymbol> functions_;
};

} // namespace epp::sema

#endif // EPP_INCLUDE_EPP_SEMA_FUNCTION_TABLE_H_
