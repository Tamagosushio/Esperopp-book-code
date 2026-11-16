#include "epp/codegen/object.h"

#include <llvm/ADT/SmallVector.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/CodeGen.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/TargetParser/Host.h>

#include <memory>
#include <stdexcept>
#include <string>

#include "llvm_module.h"

namespace epp::codegen {

std::vector<char> GenerateObject(
  const ir::Program& program,
  CodegenOptions options
) {
  throw std::logic_error("not yet implemented: object");
}

} // namespace epp::codegen
