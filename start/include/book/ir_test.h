#ifndef BOOK_IR_TEST_H_
#define BOOK_IR_TEST_H_

#include <iostream>
#include <string>
#include <string_view>
#include <variant>

#include "epp/frontend/analyze.h"
#include "epp/ir/lower.h"
#include "epp/source/source_file.h"

namespace book {

inline int TestFrontend(std::string_view text) {
  epp::source::SourceFile source{"input.epp", std::string{text}};
  const auto result = epp::frontend::AnalyzeSource(source);
  const auto* analyzed = std::get_if<epp::frontend::AnalyzedSource>(&result);
  if (!analyzed) {
    std::cerr << "frontend error\n";
    return 1;
  }
  std::cout << "frontend: ok\n";
  std::cout << "user functions: " << analyzed->program.function_ids.size() << '\n';
  return 0;
}

inline int TestIr(std::string_view text) {
  using namespace epp;
  source::SourceFile source{"input.epp", std::string{text}};
  auto analyzed = frontend::AnalyzeSource(source);
  const auto* front = std::get_if<frontend::AnalyzedSource>(&analyzed);
  if (!front) {
    std::cerr << "frontend error\n";
    return 1;
  }
  const auto lowered = ir::LowerToIr(*front);
  const auto* program = std::get_if<ir::Program>(&lowered);
  if (!program || program->functions.size() != 1) {
    std::cerr << "IR error\n";
    return 1;
  }
  const auto& f = program->functions.front();
  std::cout << "functions: " << program->functions.size() << '\n';
  for (std::size_t block = 0; block < f.blocks.size(); ++block) {
    std::cout << "b" << block << ":\n";
    for (std::size_t i = 0; i < f.expressions.size(); ++i) {
      const auto& expr = f.expressions[i];
      if (expr.block != block) continue;
      std::cout << "  %" << i << " = ";
      if (const auto* c = std::get_if<ir::IntegerConstant>(&expr.data)) {
        std::cout << "integer " << c->value;
      } else if (const auto* c = std::get_if<ir::BooleanConstant>(&expr.data)) {
        std::cout << "boolean " << (c->value ? "vera" : "malvera");
      } else if (std::holds_alternative<ir::InputExpression>(expr.data)) {
        std::cout << "input";
      } else if (const auto* length = std::get_if<ir::StringLengthExpression>(&expr.data)) {
        std::cout << "string-length %" << length->string.index;
      } else if (const auto* character = std::get_if<ir::StringCharacterExpression>(&expr.data)) {
        std::cout << "string-character %" << character->string.index << " %" << character->index.index;
      } else if (const auto* phi = std::get_if<ir::PhiExpression>(&expr.data)) {
        std::cout << "phi %" << phi->left.index << " b" << phi->left_block << " %" << phi->right.index << " b" << phi->right_block;
      } else if (const auto* load = std::get_if<ir::LoadExpression>(&expr.data)) {
        std::cout << "load v" << load->variable;
      } else if (const auto* store = std::get_if<ir::StoreExpression>(&expr.data)) {
        std::cout << "store v" << store->variable << " %" << store->value.index;
      } else if (const auto* bin = std::get_if<ir::IntegerBinaryExpression>(&expr.data)) {
        std::cout << (bin->operation == ir::IntegerOperation::kAdd ? "add" : "integer-op") << " %" << bin->left.index << " %" << bin->right.index;
      } else if (const auto* cmp = std::get_if<ir::CompareExpression>(&expr.data)) {
        std::cout << (cmp->operation == ir::Comparison::kLess ? "less" : "compare") << " %" << cmp->left.index << " %" << cmp->right.index;
      } else {
        std::cout << "other";
      }
      std::cout << '\n';
    }
    const auto& exit = f.blocks[block].exit;
    if (const auto* jump = std::get_if<ir::Jump>(&exit)) {
      std::cout << "  jump b" << jump->target << '\n';
    } else if (const auto* branch = std::get_if<ir::Branch>(&exit)) {
      std::cout << "  branch %" << branch->condition.index << " b" << branch->yes << " b" << branch->no << '\n';
    } else if (const auto* ret = std::get_if<ir::Return>(&exit)) {
      std::cout << "  return";
      if (ret->value) std::cout << " %" << ret->value->index;
      std::cout << '\n';
    } else {
      std::cout << "  unreachable\n";
    }
  }
  return 0;
}

}  // namespace book

#endif  // BOOK_IR_TEST_H_
