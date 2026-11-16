#ifndef EPP_SRC_CODEGEN_LLVM_MODULE_H_
#define EPP_SRC_CODEGEN_LLVM_MODULE_H_

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

#include <memory>

#include "epp/codegen/options.h"
#include "epp/ir/program.h"

namespace epp::codegen::detail {

// 型付きIRからLLVMのモジュールを作る。
// contextは返したModuleより長く生存させる。内部IRの不整合ではlogic_errorを送出する。
[[nodiscard]] std::unique_ptr<llvm::Module> CreateModule(
  const ir::Program& program,
  llvm::LLVMContext& context,
  CodegenOptions options
);

// 生成したモジュールをLLVMの検証器で調べる。不正なIRならlogic_errorを送出する。
void VerifyModule(const llvm::Module& module);

} // namespace epp::codegen::detail

#endif // EPP_SRC_CODEGEN_LLVM_MODULE_H_
