#ifndef EPP_INCLUDE_EPP_VALUE_TYPE_H_
#define EPP_INCLUDE_EPP_VALUE_TYPE_H_

namespace epp {

// 意味解析とIRで共有する4種類の値型。
// 整数は64ビット、文字はUnicodeスカラー値。
enum class ValueType {
  kInteger,
  kBoolean,
  kCharacter,
  kString
};

} // namespace epp

#endif // EPP_INCLUDE_EPP_VALUE_TYPE_H_
