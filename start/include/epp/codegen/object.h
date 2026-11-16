#ifndef EPP_INCLUDE_EPP_CODEGEN_OBJECT_H_
#define EPP_INCLUDE_EPP_CODEGEN_OBJECT_H_

#include <vector>

#include "epp/codegen/options.h"
#include "epp/ir/program.h"

namespace epp::codegen {

// 実行環境のCPU向けに、検証済みLLVM IRからオブジェクトのバイト列を生成する。
[[nodiscard]] std::vector<char> GenerateObject(
  const ir::Program& program,
  CodegenOptions options = {}
);

} // namespace epp::codegen

#endif // EPP_INCLUDE_EPP_CODEGEN_OBJECT_H_
