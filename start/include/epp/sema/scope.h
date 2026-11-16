#ifndef EPP_INCLUDE_EPP_SEMA_SCOPE_H_
#define EPP_INCLUDE_EPP_SEMA_SCOPE_H_

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "epp/sema/type.h"
#include "epp/source/source_range.h"

namespace epp::sema {

// 変数のシンボルを識別する番号を表す。
// 所属するScopeStack内で有効。スコープを退出しても番号を再利用しない。
struct VariableSymbolId {
  std::size_t index;
  [[nodiscard]] bool operator==(const VariableSymbolId& other) const noexcept = default;
};

// 宣言された変数について、意味解析で必要になる情報を保持する。
struct VariableSymbol {
  std::u32string normalized_identifier;
  BuiltInType type;
  bool is_constant;
  source::SourceRange declaration_range;
};

// 入れ子になったスコープと、その中で宣言された変数を管理する。
class ScopeStack {
 public:
  // 最も外側のスコープを一つ持つ状態で作成する。
  ScopeStack();
  // 現在より一つ内側のスコープに入る。
  void EnterScope();
  // 現在のスコープから出る。
  // 現在が最も外側の場合falseを返す。
  [[nodiscard]] bool ExitScope();
  // 現在のスコープで変数を宣言する。
  // 現在と同じスコープに同名の変数があれば、std::nulloptを返す。
  [[nodiscard]] std::optional<VariableSymbolId> DeclareVariable(
    VariableSymbol symbol
  );
  // 現在位置から見える同名の変数を、内側のスコープから順に探す。
  [[nodiscard]] std::optional<VariableSymbolId> LookupVariable(
    std::u32string_view normalized_identifier
  ) const;
  // IDに対応する変数情報を返す。
  // 不正なIDではout_of_rangeを送出する。
  // 変数の追加により以前の参照が無効になる可能性がある。
  [[nodiscard]] const VariableSymbol& GetVariable(
    VariableSymbolId symbol_id
  ) const;

 private:
  // 一つのスコープ内で宣言された変数IDを保持する。
  struct Scope {
    std::vector<VariableSymbolId> variable_ids;
  };
  std::vector<Scope> scopes_;
  std::vector<VariableSymbol> variables_;
};

// ブロック終了・途中return・例外のいずれでも、このスコープから退出する。
class ScopeGuard {
 public:
  explicit ScopeGuard(ScopeStack& scope) : scope_(scope) {
    scope_.EnterScope();
  }
  ~ScopeGuard() {
    static_cast<void>(scope_.ExitScope());
  }
  ScopeGuard(const ScopeGuard&) = delete;
  ScopeGuard& operator=(const ScopeGuard&) = delete;

 private:
  ScopeStack& scope_;
};

} // namespace epp::sema

#endif // EPP_INCLUDE_EPP_SEMA_SCOPE_H_
