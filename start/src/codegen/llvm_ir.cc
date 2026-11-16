#include "epp/codegen/llvm_ir.h"

#include <stdexcept>
#include <llvm/Support/raw_ostream.h>

#include <string>

#include "llvm_module.h"

namespace epp::codegen {

std::string GenerateLlvmIr(
  const ir::Program& program,
  CodegenOptions options
) {
  throw std::logic_error("not yet implemented: llvm-ir");
}

} // namespace epp::codegen
