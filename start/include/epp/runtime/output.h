#ifndef EPP_INCLUDE_EPP_RUNTIME_OUTPUT_H_
#define EPP_INCLUDE_EPP_RUNTIME_OUTPUT_H_

#include <cstdint>

extern "C" {

// SIGPIPEを無視する設定を行う。生成したmainは利用者関数を呼ぶ前に一度実行する。
void epp_output_init();
// 64ビット整数を10進数で出力バッファへ追加する。
void epp_output_integer(std::int64_t value);
// 0ならmalvera、それ以外ならveraを出力バッファへ追加する。
void epp_output_boolean(std::uint32_t value);
// Unicodeスカラー値をUTF-8へ変換して追加する。不正な値では終了コード1で停止する。
void epp_output_character(std::uint32_t scalar);
// length個のスカラー値を追加する。呼び出しの間だけ配列を参照し、途中のNULも出力する。
void epp_output_string(const std::uint32_t* scalars, std::uint64_t length);
// 保留中の出力を標準出力へ書き出す。
void epp_output_flush();
// 残ったバッファを標準出力へ書き出す。書込み失敗では終了コード1で停止する。
void epp_output_finish();

}

#endif // EPP_INCLUDE_EPP_RUNTIME_OUTPUT_H_
