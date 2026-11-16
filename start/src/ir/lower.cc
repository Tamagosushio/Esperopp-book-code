#include "epp/ir/lower.h"

#include <algorithm>
#include <optional>
#include <stdexcept>

#include <utility>
#include <vector>

#include "epp/sema/declaration.h"
#include "epp/sema/expression.h"
#include "epp/sema/function.h"
#include "epp/sema/integer_literal.h"
#include "epp/sema/name_resolution.h"

namespace epp::ir {

namespace {

IntegerOperation ToIntegerOperation(ast::BinaryOperator operation) {
  switch (operation) {
    case ast::BinaryOperator::kAdd:
      return IntegerOperation::kAdd;
    case ast::BinaryOperator::kSubtract:
      return IntegerOperation::kSubtract;
    case ast::BinaryOperator::kMultiply:
      return IntegerOperation::kMultiply;
    case ast::BinaryOperator::kDivide:
      return IntegerOperation::kDivide;
    case ast::BinaryOperator::kRemainder:
      return IntegerOperation::kRemainder;
    case ast::BinaryOperator::kBitwiseOr:
      return IntegerOperation::kBitwiseOr;
    case ast::BinaryOperator::kBitwiseXor:
      return IntegerOperation::kBitwiseXor;
    case ast::BinaryOperator::kBitwiseAnd:
      return IntegerOperation::kBitwiseAnd;
    case ast::BinaryOperator::kShiftLeft:
      return IntegerOperation::kShiftLeft;
    case ast::BinaryOperator::kShiftRight:
      return IntegerOperation::kShiftRight;
    default:
      throw std::logic_error{"invalid integer operator"};
  }
}

class FunctionLowerer {
 public:
  FunctionLowerer(
    const frontend::AnalyzedSource& source,
    const std::vector<sema::FunctionSymbolId>& functions,
    const ast::FunctionDefinition& definition,
    Function& output
  ) : source_(source), functions_(functions), output_(output) {
    current_block_ = NewBlock();
    const auto registered = sema::RegisterFunctionParameters(definition, source.ast, scope_);
    const auto& ids = std::get<std::vector<sema::VariableSymbolId>>(registered);
    for (std::size_t i = 0; i < ids.size(); ++i) {
      output_.variables.push_back(output_.parameters.at(i));
      const auto range = source.ast.GetNode(definition.parameters[i]).range;
      const auto value = Append(ParameterReference{i}, range);
      Append(StoreExpression{ids[i].index, value}, range);
    }
  }

  void Statement(ast::NodeId id) {
    const auto& node = source_.ast.GetNode(id);
    // 到達不能なソースも変換対象の検査を省かない。
    if (!Open()) {
      current_block_ = NewBlock();
    }
    if (const auto* block = std::get_if<ast::BlockStatement>(&node.data)) {
      sema::ScopeGuard scope{scope_};
      for (const auto child : block->statements) {
        Statement(child);
      }
    } else if (const auto* declaration = std::get_if<ast::LocalDeclarationStatement>(&node.data)) {
      const auto analyzed = sema::AnalyzeDeclarationComponents(declaration->components);
      const auto& info = std::get<sema::AnalyzedDeclaration>(analyzed);
      const auto type = info.type;
      // 初期化式からは、新しい名前をまだ見せない。
      const auto value
        = declaration->initializer
        ? Lower(*declaration->initializer)
        : DefaultValue(type, node.range);
      const auto variable = scope_.DeclareVariable(sema::VariableSymbol{
        info.normalized_identifier,
        info.type,
        info.is_constant,
        info.name_range
      });
      if (!variable || variable->index != output_.variables.size()) {
        throw std::logic_error{"invalid analyzed declaration"};
      }
      output_.variables.push_back(type);
      Append(StoreExpression{variable->index, value}, node.range);
    } else if (const auto* assignment = std::get_if<ast::AssignmentStatement>(&node.data)) {
      const auto& target = std::get<ast::NameExpression>(source_.ast.GetNode(assignment->target).data);
      const auto variable = scope_.LookupVariable(target.normalized_identifier);
      if (!variable || scope_.GetVariable(*variable).is_constant) {
        throw std::logic_error{"invalid analyzed assignment"};
      }
      const auto value = Lower(assignment->value);
      Append(StoreExpression{variable->index, value}, node.range);
    } else if (const auto* call = std::get_if<ast::CallExpression>(&node.data)) {
      const auto resolved = sema::ResolveCallTarget(id, source_.ast, source_.program.function_table);
      const auto target = std::get<sema::FunctionSymbolId>(resolved);
      const auto& function = source_.program.function_table.GetFunction(target);
      if (function.builtin == sema::BuiltInFunction::kOutput) {
        const auto argument = call->arguments.at(0);
        const auto analyzed = sema::AnalyzeExpression(
          argument, source_.ast, scope_, source_.program.function_table
        );
        const auto type = std::get<sema::BuiltInType>(analyzed);
        const auto value = Lower(argument);
        Append(OutputInstruction{value, type}, node.range);
      } else if (!function.return_type) {
        Lower(id);
      } else {
        throw LoweringError{LoweringErrorKind::kInvalidStatement, node.range};
      }
    } else if (const auto* result = std::get_if<ast::ReturnStatement>(&node.data)) {
      std::optional<ExpressionId> value;
      if (result->value) {
        value = Lower(*result->value);
      }
      output_.blocks[current_block_].exit = Return{value, node.range};
    } else if (const auto* conditional = std::get_if<ast::IfStatement>(&node.data)) {
      const auto after = NewBlock();
      for (const auto& branch : conditional->branches) {
        const auto condition = Lower(branch.condition);
        const auto yes = NewBlock();
        const auto no = NewBlock();
        output_.blocks[current_block_].exit = Branch{condition, yes, no};
        current_block_ = yes;
        Statement(branch.body);
        JumpIfOpen(after);
        current_block_ = no;
      }
      if (conditional->else_body) {
        Statement(*conditional->else_body);
      }
      JumpIfOpen(after);
      current_block_ = after;
    } else if (const auto* loop = std::get_if<ast::WhileStatement>(&node.data)) {
      const auto condition_block = NewBlock();
      const auto body = NewBlock();
      const auto after = NewBlock();
      JumpIfOpen(condition_block);
      current_block_ = condition_block;
      const auto condition = Lower(loop->condition);
      output_.blocks[current_block_].exit = Branch{condition, body, after};
      loop_targets_.push_back({condition_block, after});
      current_block_ = body;
      Statement(loop->body);
      JumpIfOpen(condition_block);
      loop_targets_.pop_back();
      current_block_ = after;
    } else if (std::holds_alternative<ast::BreakStatement>(node.data) || std::holds_alternative<ast::ContinueStatement>(node.data)) {
      if (loop_targets_.empty()) {
        throw std::logic_error{"analyzed loop control outside loop"};
      }
      const bool stop = std::holds_alternative<ast::BreakStatement>(node.data);
      output_.blocks[current_block_].exit = Jump{
        stop ? loop_targets_.back().exit_block : loop_targets_.back().condition_block
      };
    } else {
      throw LoweringError{LoweringErrorKind::kInvalidStatement, node.range};
    }
  }

  void Finish() {
    if (!output_.return_type && Open()) {
      output_.blocks[current_block_].exit = Return{std::nullopt, output_.range};
    }
  }

 private:
  struct LoopTargets {
    std::size_t condition_block;
    std::size_t exit_block;
  };

  ExpressionId Lower(ast::NodeId id) {
    const auto& node = source_.ast.GetNode(id);
    if (const auto* literal = std::get_if<ast::IntegerLiteralExpression>(&node.data)) {
      return Constant(*literal, false, node.range);
    }
    if (const auto* literal = std::get_if<ast::BooleanLiteralExpression>(&node.data)) {
      return Append(BooleanConstant{literal->value}, node.range);
    }
    if (const auto* literal = std::get_if<ast::CharacterLiteralExpression>(&node.data)) {
      return Append(CharacterConstant{literal->value}, node.range);
    }
    if (const auto* literal = std::get_if<ast::StringLiteralExpression>(&node.data)) {
      return Append(StringConstant{literal->value}, node.range);
    }
    if (const auto* unary = std::get_if<ast::UnaryExpression>(&node.data)) {
      const auto* literal = std::get_if<ast::IntegerLiteralExpression>(
        &source_.ast.GetNode(unary->operand).data
      );
      if (unary->operator_kind == ast::UnaryOperator::kMinus && literal) {
        return Constant(*literal, true, node.range);
      }
      const auto operand = Lower(unary->operand);
      switch (unary->operator_kind) {
        case ast::UnaryOperator::kPlus:
          return operand;
        case ast::UnaryOperator::kMinus: {
          const auto zero = Append(IntegerConstant{0}, node.range);
          return Append(
            IntegerBinaryExpression{IntegerOperation::kSubtract, zero, operand},
            node.range
          );
        }
        case ast::UnaryOperator::kBitwiseNot: {
          const auto mask = Append(IntegerConstant{-1}, node.range);
          return Append(
            IntegerBinaryExpression{IntegerOperation::kBitwiseXor, operand, mask},
            node.range
          );
        }
        case ast::UnaryOperator::kLogicalNot:
          return Append(NotExpression{operand}, node.range);
      }
    }
    if (const auto* name = std::get_if<ast::NameExpression>(&node.data)) {
      const auto variable = scope_.LookupVariable(name->normalized_identifier);
      if (!variable) {
        throw std::logic_error{"unknown analyzed variable"};
      }
      return Append(LoadExpression{variable->index}, node.range);
    }
    if (const auto* binary = std::get_if<ast::BinaryExpression>(&node.data)) {
      using Op = ast::BinaryOperator;
      const auto op = binary->operator_kind;
      std::optional<Comparison> comparison;
      switch (op) {
        case Op::kEqual:
          comparison = Comparison::kEqual;
          break;
        case Op::kNotEqual:
          comparison = Comparison::kNotEqual;
          break;
        case Op::kLess:
          comparison = Comparison::kLess;
          break;
        case Op::kLessEqual:
          comparison = Comparison::kLessEqual;
          break;
        case Op::kGreater:
          comparison = Comparison::kGreater;
          break;
        case Op::kGreaterEqual:
          comparison = Comparison::kGreaterEqual;
          break;
        case Op::kAdd:
        case Op::kSubtract:
        case Op::kMultiply:
        case Op::kDivide:
        case Op::kRemainder:
        case Op::kBitwiseOr:
        case Op::kBitwiseXor:
        case Op::kBitwiseAnd:
        case Op::kShiftLeft:
        case Op::kShiftRight:
        case Op::kLogicalAnd:
        case Op::kLogicalOr:
          break;
        default:
          throw LoweringError{LoweringErrorKind::kInvalidExpression, node.range};
      }
      const auto left = Lower(binary->left_operand);
      if (op == Op::kLogicalAnd || op == Op::kLogicalOr) {
        const auto left_block = current_block_;
        const auto right_block = NewBlock();
        const auto after = NewBlock();
        output_.blocks[current_block_].exit = op == Op::kLogicalAnd
          ? Branch{left, right_block, after}
          : Branch{left, after, right_block};
        current_block_ = right_block;
        const auto right = Lower(binary->right_operand);
        const auto right_end = current_block_;
        JumpIfOpen(after);
        current_block_ = after;
        return Append(PhiExpression{left, left_block, right, right_end}, node.range);
      }
      const auto right = Lower(binary->right_operand);
      if (comparison) {
        return Append(CompareExpression{*comparison, left, right}, node.range);
      }
      return Append(
        IntegerBinaryExpression{ToIntegerOperation(op), left, right},
        node.range
      );
    }
    if (const auto* call = std::get_if<ast::CallExpression>(&node.data)) {
      const auto resolved = sema::ResolveCallTarget(
        id, source_.ast, source_.program.function_table
      );
      const auto target = std::get<sema::FunctionSymbolId>(resolved);
      const auto builtin
        = source_.program.function_table.GetFunction(target).builtin;
      if (builtin == sema::BuiltInFunction::kInput) {
        return Append(InputExpression{}, node.range);
      }
      if (builtin == sema::BuiltInFunction::kStringLength) {
        const auto string = Lower(call->arguments.at(0));
        return Append(StringLengthExpression{string}, node.range);
      }
      if (builtin == sema::BuiltInFunction::kStringCharacter) {
        const auto string = Lower(call->arguments.at(0));
        const auto index = Lower(call->arguments.at(1));
        return Append(StringCharacterExpression{string, index}, node.range);
      }
      if (builtin == sema::BuiltInFunction::kTapeStep) {
        return Append(
          TapeStepExpression{Lower(call->arguments.at(0)), Lower(call->arguments.at(1))},
          node.range
        );
      }
      const auto found = std::find(functions_.begin(), functions_.end(), target);
      if (found == functions_.end()) {
        throw std::logic_error{"missing analyzed function"};
      }
      CallExpression result{static_cast<std::size_t>(found - functions_.begin()), {}};
      for (const auto argument : call->arguments) {
        result.arguments.push_back(Lower(argument));
      }
      return Append(std::move(result), node.range);
    }
    throw LoweringError{LoweringErrorKind::kInvalidExpression, node.range};
  }

  template <typename T>
  ExpressionId Append(T data, source::SourceRange range) {
    throw std::logic_error("not yet implemented: append");
  }

  std::size_t NewBlock() {
    throw std::logic_error("not yet implemented: new-block");
  }

  bool Open() const {
    return std::holds_alternative<std::monostate>(
      output_.blocks[current_block_].exit
    );
  }

  void JumpIfOpen(std::size_t target) {
    throw std::logic_error("not yet implemented: jump-if-open");
  }

  ExpressionId DefaultValue(Type type, source::SourceRange range) {
    switch (type) {
      case Type::kInteger:
        return Append(IntegerConstant{0}, range);
      case Type::kBoolean:
        return Append(BooleanConstant{false}, range);
      case Type::kCharacter:
        return Append(CharacterConstant{U'\0'}, range);
      case Type::kString:
        return Append(StringConstant{U""}, range);
    }
    throw std::logic_error{"invalid IR type"};
  }

  ExpressionId Constant(
    const ast::IntegerLiteralExpression& literal,
    bool negate,
    source::SourceRange range
  ) {
    const auto value = sema::FormIntegerLiteralValue(literal.digits, negate);
    if (!value) {
      throw std::logic_error{"out-of-range analyzed literal"};
    }
    return Append(IntegerConstant{*value}, range);
  }

  const frontend::AnalyzedSource& source_;
  const std::vector<sema::FunctionSymbolId>& functions_;
  sema::ScopeStack scope_;
  Function& output_;
  std::size_t current_block_ = 0;
  std::vector<LoopTargets> loop_targets_;
};

} // namespace

LoweringResult LowerToIr(const frontend::AnalyzedSource& source) {
  throw std::logic_error("not yet implemented: lower-to-ir");
}

} // namespace epp::ir
