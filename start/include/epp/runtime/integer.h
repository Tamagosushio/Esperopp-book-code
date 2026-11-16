#ifndef EPP_INCLUDE_EPP_RUNTIME_INTEGER_H_
#define EPP_INCLUDE_EPP_RUNTIME_INTEGER_H_

// 除数0または範囲外のシフト量を診断し、終了コード1で停止する。
// 標準エラーへの書込みに失敗しても停止する。出力バッファは再書込みしない。
extern "C" [[noreturn]] void epp_integer_error();

#endif // EPP_INCLUDE_EPP_RUNTIME_INTEGER_H_
