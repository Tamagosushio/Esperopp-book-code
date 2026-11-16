#ifndef EPP_INCLUDE_EPP_CODEGEN_LLVM_IR_H_
#define EPP_INCLUDE_EPP_CODEGEN_LLVM_IR_H_

#include <string>

#include "epp/codegen/options.h"
#include "epp/ir/program.h"

namespace epp::codegen {

// 型つきIRから、検証済みLLVM IRのテキストを生成する。
[[nodiscard]] std::string GenerateLlvmIr(
  const ir::Program& program,
  CodegenOptions options = {}
);

} // namespace epp::codegen

#endif // EPP_INCLUDE_EPP_CODEGEN_LLVM_IR_H_
