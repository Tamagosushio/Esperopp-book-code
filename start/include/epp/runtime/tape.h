#ifndef EPP_INCLUDE_EPP_RUNTIME_TAPE_H_
#define EPP_INCLUDE_EPP_RUNTIME_TAPE_H_

#include <cstdint>

// 符号付き64ビット整数のテープを実行単位で共有する。
extern "C" {

// 現在のセルへvalueを書き、rightが0なら左、それ以外なら右へ移動する。
// 移動先の整数を返す。初めて読むセルは0。確保失敗では終了コード1で停止する。
std::int64_t epp_tape_step(std::int64_t value, std::uint32_t right);

// 確保したセルを解放し、先頭セルとヘッドを初期状態へ戻す。
void epp_tape_finish();

}

#endif // EPP_INCLUDE_EPP_RUNTIME_TAPE_H_
