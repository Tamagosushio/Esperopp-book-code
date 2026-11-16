#ifndef EPP_INCLUDE_EPP_AST_NODE_H_
#define EPP_INCLUDE_EPP_AST_NODE_H_

#include <cstddef>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "epp/source/source_range.h"

namespace epp::ast {

// AST内のノードを識別する番号を表す。
struct NodeId {
  std::size_t index;
  [[nodiscard]] bool operator==(const NodeId& other) const noexcept = default;
};

// 整数リテラル式を表す。
// 符号を含まない10進数字列。
struct IntegerLiteralExpression {
  std::string digits;
};

// 真理値リテラル式を表す。
struct BooleanLiteralExpression {
  bool value;
};

// 文字リテラル式を表す。
// 一つのUnicodeスカラー値。
struct CharacterLiteralExpression {
  char32_t value;
};

// 文字列リテラル式を表す。
// 字句解析でエスケープを展開したスカラー値の列。
struct StringLiteralExpression {
  std::u32string value;
};

// 名前を参照する式を表す。
struct NameExpression {
  std::u32string normalized_identifier;
};

// 単項式で使用する演算子を表す。
enum class UnaryOperator {
  kPlus,
  kMinus,
  kBitwiseNot,
  kLogicalNot,
};

// 二項式で使用する演算子を表す。
enum class BinaryOperator {
  kAdd,
  kSubtract,
  kMultiply,
  kDivide,
  kRemainder,
  kBitwiseOr,
  kBitwiseXor,
  kBitwiseAnd,
  kEqual,
  kNotEqual,
  kLess,
  kLessEqual,
  kGreater,
  kGreaterEqual,
  kShiftLeft,
  kShiftRight,
  kLogicalAnd,
  kLogicalOr,
};

// 一つのオペランドへ演算子を適用する式を表す。
struct UnaryExpression {
  UnaryOperator operator_kind;
  NodeId operand;
};

// 二つのオペランドへ演算子を適用する式を表す。
struct BinaryExpression {
  BinaryOperator operator_kind;
  NodeId left_operand;
  NodeId right_operand;
};

// 関数の呼び出し先と渡す実引数を表す式。
struct CallExpression {
  NodeId callee;
  std::vector<NodeId> arguments;
};

// 宣言に現れる正規化済みの識別子を表す。
struct DeclarationIdentifier {
  std::u32string normalized_identifier;
};

// 宣言で使用する修飾子を表す。
enum class DeclarationModifier {
  kConst,
};

// 宣言を構成する要素のデータを表す。
using DeclarationComponentData = std::variant<
  DeclarationIdentifier,
  DeclarationModifier
>;

// 宣言の識別子または修飾子と、ソース上の範囲を表す。
struct DeclarationComponent {
  source::SourceRange range;
  DeclarationComponentData data;
};

// ローカル変数の宣言要素と初期化式を表す。
// 初期化式は省略可能。
struct LocalDeclarationStatement {
  std::vector<DeclarationComponent> components;
  std::optional<NodeId> initializer;
};

// 値を返さない関数を示すvakaを表す。
struct VoidReturnMarker {};

// 関数ヘッダを構成する要素のデータを表す。
using FunctionHeadComponentData = std::variant<
  DeclarationIdentifier,
  VoidReturnMarker
>;

// 関数ヘッダの識別子またはvakaと、ソース上の範囲を表す。
struct FunctionHeadComponent {
  source::SourceRange range;
  FunctionHeadComponentData data;
};

// 関数パラメータを構成する宣言要素を表す。
struct FunctionParameter {
  std::vector<DeclarationComponent> components;
};

// 代入先と代入する値を表す。
struct AssignmentStatement {
  NodeId target;
  NodeId value;
};

// 関数から呼び出し元に戻る文を表す。
// 戻り値がない場合、valueはstd::nulloptになる。
// 値の省略はvaka関数に限り、意味解析で検査する。
struct ReturnStatement {
  std::optional<NodeId> value;
};

// 波括弧で囲まれた文ブロックを表す。
struct BlockStatement {
  std::vector<NodeId> statements;
};

// seまたはalie seの条件式と文ブロックを表す。
struct IfBranch {
  NodeId condition;
  NodeId body;
};

// se文の分岐とalie文ブロックを表す。
// alie文ブロックは省略可能。
struct IfStatement {
  std::vector<IfBranch> branches;
  std::optional<NodeId> else_body;
};

// 条件を満たす間繰り返すdum文を表す。
struct WhileStatement {
  NodeId condition;
  NodeId body;
};

// ループを終了するrompu文を表す。
struct BreakStatement {};

// ループを続けるdaŭrigu文を表す。
struct ContinueStatement {};

// 関数ヘッダ、パラメータ列、文ブロックを持つ関数定義を表す。
struct FunctionDefinition {
  std::vector<FunctionHeadComponent> head_components;
  std::vector<NodeId> parameters;
  NodeId body;
};

// ソースファイルに含まれる関数定義を表す。
struct CompilationUnit {
  std::vector<NodeId> function_definitions;
};

// ASTノードが保持できるデータの種類。
using NodeData = std::variant<
  IntegerLiteralExpression,
  BooleanLiteralExpression,
  CharacterLiteralExpression,
  StringLiteralExpression,
  NameExpression,
  UnaryExpression,
  BinaryExpression,
  CallExpression,
  AssignmentStatement,
  ReturnStatement,
  BlockStatement,
  LocalDeclarationStatement,
  IfStatement,
  WhileStatement,
  BreakStatement,
  ContinueStatement,
  FunctionParameter,
  FunctionDefinition,
  CompilationUnit
>;

// ソースの範囲と、ノードのデータをまとめて保持する。
struct Node {
  source::SourceRange range;
  NodeData data;
};

// ASTノードを一括管理する。
class Ast {
 public:
  // ノードを追加し、そのノードを識別するNodeIdを返す。
  [[nodiscard]] NodeId AddNode(Node node);
  // NodeIdに対応するノードの参照を返す。
  // 不正な番号ではout_of_rangeを送出する。
  [[nodiscard]] const Node& GetNode(NodeId node_id) const;
  // 現在保持しているノード数を返す。
  [[nodiscard]] std::size_t NodeCount() const noexcept;

 private:
  std::vector<Node> nodes_;
};

} // namespace epp::ast

#endif // EPP_INCLUDE_EPP_AST_NODE_H_
