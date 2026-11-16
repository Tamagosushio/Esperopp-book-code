#ifndef EPP_INCLUDE_EPP_CODEGEN_OPTIONS_H_
#define EPP_INCLUDE_EPP_CODEGEN_OPTIONS_H_

namespace epp::codegen {

// LLVM IRとオブジェクトの生成に共通する設定。
struct CodegenOptions {
  // epp_entryを呼び、その戻り値を返すmainも生成する。
  bool emit_main = false;
};

} // namespace epp::codegen

#endif // EPP_INCLUDE_EPP_CODEGEN_OPTIONS_H_
