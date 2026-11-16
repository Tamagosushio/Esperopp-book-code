#include "epp/sema/program.h"

#include <stdexcept>
#include <optional>
#include <utility>

#include "epp/sema/function.h"

namespace epp::sema {

ProgramAnalysisResult AnalyzeProgram(
  const ast::CompilationUnit& compilation_unit,
  const ast::Ast& ast
) {
  throw std::logic_error("not yet implemented: analyze-program");
}

} // namespace epp::sema
