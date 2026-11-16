#ifndef BOOK_LLVM_TEST_H_
#define BOOK_LLVM_TEST_H_

#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <variant>

#include "epp/codegen/llvm_ir.h"
#include "epp/codegen/object.h"
#include "epp/frontend/analyze.h"
#include "epp/ir/lower.h"
#include "epp/source/source_file.h"

namespace book {

inline std::optional<epp::ir::Program> MakeProgram(std::string_view text) {
  epp::source::SourceFile source{"input.epp", std::string{text}};
  auto result = epp::frontend::AnalyzeSource(source);
  const auto* front = std::get_if<epp::frontend::AnalyzedSource>(&result);
  if (!front) return std::nullopt;
  auto lowered = epp::ir::LowerToIr(*front);
  const auto* program = std::get_if<epp::ir::Program>(&lowered);
  if (!program) return std::nullopt;
  return *program;
}

inline int TestLlvm(std::string_view text) {
  const auto program = MakeProgram(text);
  if (!program) {
    std::cerr << "analysis error\n";
    return 1;
  }
  std::cout << epp::codegen::GenerateLlvmIr(*program);
  return 0;
}

inline int TestObject(std::string_view text) {
  const auto program = MakeProgram(text);
  if (!program) {
    std::cerr << "analysis error\n";
    return 1;
  }
  const auto bytes = epp::codegen::GenerateObject(*program, {.emit_main = true});
  if (bytes.empty()) {
    std::cerr << "empty object\n";
    return 1;
  }
  std::cout << "object: generated\n";
  return 0;
}

}  // namespace book

#endif  // BOOK_LLVM_TEST_H_
