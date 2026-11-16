#ifndef BOOK_IR_PRINT_H_
#define BOOK_IR_PRINT_H_

#include <iostream>
#include <variant>

#include "epp/ir/program.h"

namespace book {

// 単一関数のIRをテスト用の表記で表示する。
inline void PrintIr(const epp::ir::Program& program) {
  using namespace epp;
  const auto& f = program.functions.front();
  std::cout << "functions: " << program.functions.size() << '\n';
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
}

} // namespace book

#endif // BOOK_IR_PRINT_H_
