#ifndef EPP_INCLUDE_EPP_IR_PROGRAM_H_
#define EPP_INCLUDE_EPP_IR_PROGRAM_H_

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "epp/source/source_range.h"
#include "epp/value_type.h"

namespace epp::ir {

// 意味解析と共有する値型。
using Type = ValueType;

// 所属するFunctionのexpressions内の位置。別の関数やASTのNodeIdとは交換できない。
struct ExpressionId {
  std::size_t index;
};

// 64ビット整数のコンパイル時定数。算術結果は64ビットの範囲で折り返す。
struct IntegerConstant {
  std::int64_t value;
};
// 真理値のコンパイル時定数。
struct BooleanConstant {
  bool value;
};
// Unicodeスカラー値のコンパイル時定数。
struct CharacterConstant {
  char32_t value;
};
// Unicodeスカラー値の文字列のコンパイル時定数。
// 末尾NULに頼らず、途中のNULも文字として保持する。
struct StringConstant {
  std::u32string value;
};

// 関数のparameters内の位置で指定する、呼び出し時の引数。
struct ParameterReference {
  std::size_t index;
};
// variables内のスロットから値を読む命令。
struct LoadExpression {
  std::size_t variable;
};
// variables内のスロットへ値を保存する命令。この命令の結果は値として使わない。
struct StoreExpression {
  std::size_t variable;
  ExpressionId value;
};
// 整数の算術・ビット演算・シフトの種類。
// 除算は0方向へ切り捨て、右シフトは符号ビットを補う。
enum class IntegerOperation {
  kAdd,
  kSubtract,
  kMultiply,
  kDivide,
  kRemainder,
  kBitwiseOr,
  kBitwiseXor,
  kBitwiseAnd,
  kShiftLeft,
  kShiftRight
};

// 二つの整数値へ演算を適用する命令。先に作った値の番号を使う。
// 最小値/-1の商は最小値、剰余は0。加減乗算と左シフトは下位64ビットを使う。
// 除数0と0〜63以外のシフト量では、生成プログラムを終了コード1で停止する。
struct IntegerBinaryExpression {
  IntegerOperation operation;
  ExpressionId left;
  ExpressionId right;
};
// 比較の種類。文字列は等値・非等値だけを使い、整数の順序は符号付きで比較する。
enum class Comparison {
  kEqual,
  kNotEqual,
  kLess,
  kLessEqual,
  kGreater,
  kGreaterEqual
};
// 二つの値を比較し、真理値を返す命令。
struct CompareExpression {
  Comparison operation;
  ExpressionId left;
  ExpressionId right;
};
// 真理値を反転する命令。
struct NotExpression {
  ExpressionId operand;
};
// Program内の関数の位置と、宣言順に渡す実引数の値。
// 呼び出す関数がvakaなら、この命令の結果は値として使わない。
struct CallExpression {
  std::size_t function_index;
  std::vector<ExpressionId> arguments;
};
// 共有テープへ整数を書き、左右どちらかへ移動して、移動先の整数を読む命令。
// valueはentjera、rightはbuleaの値。rightが真なら右、偽なら左へ1セル移動する。
struct TapeStepExpression {
  ExpressionId value;
  ExpressionId right;
};
// 標準入力から一行を読み、文字列を返す命令。
struct InputExpression {};
// 文字列のUnicodeスカラー値の個数を返す命令。
struct StringLengthExpression {
  ExpressionId string;
};
// 指定位置のUnicodeスカラー値を返す命令。
struct StringCharacterExpression {
  ExpressionId string;
  ExpressionId index;
};
// 一つの値を指定した型で表示する命令。改行も文字・文字列の値として渡す。
struct OutputInstruction {
  ExpressionId value;
  Type type;
};
// 短絡評価の二経路を合流させる命令。値と直前のブロックを組で指定する。
struct PhiExpression {
  ExpressionId left;
  std::size_t left_block;
  ExpressionId right;
  std::size_t right_block;
};

// 命令、ソース範囲、所属ブロック。配列では依存する値を先に置く。
struct Expression {
  std::variant<
    IntegerConstant,
    BooleanConstant,
    CharacterConstant,
    StringConstant,
    ParameterReference,
    LoadExpression,
    StoreExpression,
    IntegerBinaryExpression,
    CompareExpression,
    NotExpression,
    CallExpression,
    PhiExpression,
    OutputInstruction,
    InputExpression,
    StringLengthExpression,
    StringCharacterExpression,
    TapeStepExpression
  > data;
  source::SourceRange range;
  std::size_t block;
};

// 関数を終了するブロックの出口。vakaではvalueがnulloptになる。
struct Return {
  std::optional<ExpressionId> value;
  source::SourceRange range;
};

// 指定したブロックへ無条件に進む出口。
struct Jump {
  std::size_t target;
};
// conditionが真ならyes、偽ならnoのブロックへ進む出口。
struct Branch {
  ExpressionId condition;
  std::size_t yes;
  std::size_t no;
};
// monostateは到達不能な末端。意味解析済みの関数では、実行がここへ到達しない。
struct Block {
  std::variant<std::monostate, Return, Jump, Branch> exit;
};
// 署名、変数スロット、依存順の命令と基本ブロックを持つ関数。
// 各番号はこの関数内だけで有効。ブロックの出口の後には命令を置かない。
// return_typeがnulloptなら値を返さない。仮引数と変数には値型だけを使う。
struct Function {
  std::optional<Type> return_type = Type::kInteger;
  std::vector<Type> parameters;
  std::vector<Type> variables;
  std::vector<Expression> expressions;
  std::vector<Block> blocks;
  source::SourceRange range;
};

// 定義順の利用者関数と、実行開始関数の位置。
struct Program {
  std::vector<Function> functions;
  std::size_t entry_point;
};

} // namespace epp::ir

#endif // EPP_INCLUDE_EPP_IR_PROGRAM_H_
