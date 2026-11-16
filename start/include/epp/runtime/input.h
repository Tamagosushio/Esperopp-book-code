#ifndef EPP_INCLUDE_EPP_RUNTIME_INPUT_H_
#define EPP_INCLUDE_EPP_RUNTIME_INPUT_H_

#include <cstdint>

extern "C" {

// UTF-8の一行を読み、内容とUnicodeスカラー値の個数を返す。
// 返した領域はepp_input_finishまで有効。入力失敗時は診断を出して停止する。
void epp_input_line(const std::uint32_t** scalars, std::uint64_t* length);

// 入力した文字列の領域を解放する。利用者の関数が戻った後に呼ぶ。
void epp_input_finish();

}

#endif // EPP_INCLUDE_EPP_RUNTIME_INPUT_H_
