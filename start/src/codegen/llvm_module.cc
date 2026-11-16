#include "llvm_module.h"

#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/raw_ostream.h>

#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace epp::codegen::detail {

namespace {

// Unicode scalar列の内容を比較する。
llvm::Function* StringEqual(
  llvm::Module& module,
  llvm::StructType* string_type
) {
  if (auto* existing = module.getFunction("epp_string_equal")) {
    return existing;
  }
  auto& context = module.getContext();
  llvm::IRBuilder<> b{context};
  auto* type = llvm::FunctionType::get(b.getInt1Ty(), {string_type, string_type}, false);
  auto* f = llvm::Function::Create(type, llvm::Function::InternalLinkage, "epp_string_equal", module);
  auto* entry = llvm::BasicBlock::Create(context, "entry", f);
  auto* test = llvm::BasicBlock::Create(context, "test", f);
  auto* compare = llvm::BasicBlock::Create(context, "compare", f);
  auto* step = llvm::BasicBlock::Create(context, "step", f);
  auto* yes = llvm::BasicBlock::Create(context, "equal", f);
  auto* no = llvm::BasicBlock::Create(context, "different", f);
  b.SetInsertPoint(entry);
  auto* left = b.CreateExtractValue(f->getArg(0), 0);
  auto* right = b.CreateExtractValue(f->getArg(1), 0);
  auto* length = b.CreateExtractValue(f->getArg(0), 1);
  auto* other_length = b.CreateExtractValue(f->getArg(1), 1);
  b.CreateCondBr(b.CreateICmpEQ(length, other_length), test, no);
  b.SetInsertPoint(test);
  auto* index = b.CreatePHI(b.getInt64Ty(), 2, "index");
  index->addIncoming(b.getInt64(0), entry);
  b.CreateCondBr(b.CreateICmpEQ(index, length), yes, compare);
  b.SetInsertPoint(compare);
  auto* a = b.CreateLoad(b.getInt32Ty(), b.CreateInBoundsGEP(b.getInt32Ty(), left, index));
  auto* c = b.CreateLoad(b.getInt32Ty(), b.CreateInBoundsGEP(b.getInt32Ty(), right, index));
  b.CreateCondBr(b.CreateICmpEQ(a, c), step, no);
  b.SetInsertPoint(step);
  auto* next = b.CreateAdd(index, b.getInt64(1));
  index->addIncoming(next, step);
  b.CreateBr(test);
  b.SetInsertPoint(yes);
  b.CreateRet(b.getTrue());
  b.SetInsertPoint(no);
  b.CreateRet(b.getFalse());
  return f;
}

llvm::CmpInst::Predicate Predicate(ir::Comparison comparison) {
  switch (comparison) {
    case ir::Comparison::kEqual:
      return llvm::CmpInst::ICMP_EQ;
    case ir::Comparison::kNotEqual:
      return llvm::CmpInst::ICMP_NE;
    case ir::Comparison::kLess:
      return llvm::CmpInst::ICMP_SLT;
    case ir::Comparison::kLessEqual:
      return llvm::CmpInst::ICMP_SLE;
    case ir::Comparison::kGreater:
      return llvm::CmpInst::ICMP_SGT;
    case ir::Comparison::kGreaterEqual:
      return llvm::CmpInst::ICMP_SGE;
  }
  throw std::logic_error{"invalid comparison"};
}

void RequireIntegerCondition(llvm::IRBuilder<>& builder, llvm::Value* condition) {
  auto* function = builder.GetInsertBlock()->getParent();
  auto* valid = llvm::BasicBlock::Create(builder.getContext(), "integer.valid", function);
  auto* invalid = llvm::BasicBlock::Create(builder.getContext(), "integer.invalid", function);
  builder.CreateCondBr(condition, valid, invalid);
  builder.SetInsertPoint(invalid);
  auto* signature = llvm::FunctionType::get(builder.getVoidTy(), false);
  auto* call = builder.CreateCall(function->getParent()->getOrInsertFunction("epp_integer_error", signature));
  call->setDoesNotReturn();
  builder.CreateUnreachable();
  builder.SetInsertPoint(valid);
}

llvm::Value* EmitIntegerOperation(
  llvm::IRBuilder<>& builder,
  ir::IntegerOperation operation,
  llvm::Value* left,
  llvm::Value* right
) {
  throw std::logic_error("not yet implemented: integer-operation");
}

llvm::Constant* CreateStringConstant(
  llvm::Module& module,
  llvm::StructType* string_type,
  std::u32string_view value
) {
  llvm::IRBuilder<> builder{module.getContext()};
  std::vector<std::uint32_t> scalars(value.begin(), value.end());
  // 空でも有効な静的配列を持つ。
  if (scalars.empty()) {
    scalars.push_back(0);
  }
  auto* contents = llvm::ConstantDataArray::get(module.getContext(), scalars);
  auto* storage = new llvm::GlobalVariable(
    module, contents->getType(), true, llvm::GlobalValue::PrivateLinkage, contents, "text"
  );
  storage->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Global);
  return llvm::ConstantStruct::get(string_type, {storage, builder.getInt64(value.size())});
}

llvm::Value* EmitOutput(
  llvm::Module& module,
  llvm::IRBuilder<>& builder,
  const ir::OutputInstruction& data,
  llvm::Value* argument,
  llvm::Type* expected_type
) {
  if (argument->getType() != expected_type) {
    throw std::logic_error{"invalid IR output type"};
  }
  const char* name = nullptr;
  switch (data.type) {
    case ir::Type::kInteger:
      name = "epp_output_integer";
      break;
    case ir::Type::kBoolean:
      name = "epp_output_boolean";
      argument = builder.CreateZExt(argument, builder.getInt32Ty());
      break;
    case ir::Type::kCharacter:
      name = "epp_output_character";
      break;
    case ir::Type::kString:
      name = "epp_output_string";
      break;
  }
  std::vector<llvm::Value*> arguments;
  if (data.type == ir::Type::kString) {
    arguments = {builder.CreateExtractValue(argument, 0), builder.CreateExtractValue(argument, 1)};
  } else {
    arguments = {argument};
  }
  std::vector<llvm::Type*> parameters;
  for (auto* arg : arguments) {
    parameters.push_back(arg->getType());
  }
  auto* signature = llvm::FunctionType::get(builder.getVoidTy(), parameters, false);
  return builder.CreateCall(module.getOrInsertFunction(name, signature), arguments);
}

void EmitMain(
  llvm::Module& module,
  llvm::IRBuilder<>& builder,
  llvm::Function* entry
) {
  auto* main_function = llvm::Function::Create(
    llvm::FunctionType::get(builder.getInt32Ty(), false), llvm::Function::ExternalLinkage, "main", module
  );
  builder.SetInsertPoint(llvm::BasicBlock::Create(module.getContext(), "entry", main_function));
  auto* runtime_type = llvm::FunctionType::get(builder.getVoidTy(), false);
  builder.CreateCall(module.getOrInsertFunction("epp_output_init", runtime_type));
  auto* result = builder.CreateCall(entry, {}, "result");
  builder.CreateCall(module.getOrInsertFunction("epp_input_finish", runtime_type));
  builder.CreateCall(module.getOrInsertFunction("epp_tape_finish", runtime_type));
  builder.CreateCall(module.getOrInsertFunction("epp_output_finish", runtime_type));
  builder.CreateRet(builder.CreateTrunc(result, builder.getInt32Ty()));
}

} // namespace

std::unique_ptr<llvm::Module> CreateModule(
  const ir::Program& program,
  llvm::LLVMContext& context,
  CodegenOptions options
) {
  if (program.entry_point >= program.functions.size()
    || !program.functions[program.entry_point].parameters.empty()
    || program.functions[program.entry_point].return_type != ir::Type::kInteger) {
    throw std::logic_error{"invalid IR entry point"};
  }
  auto module = std::make_unique<llvm::Module>("epp", context);
  llvm::IRBuilder<> builder{context};
  auto* string_type = llvm::StructType::get(builder.getPtrTy(), builder.getInt64Ty());
  const auto type = [&](ir::Type input) -> llvm::Type* {
    switch (input) {
      case ir::Type::kInteger:
        return builder.getInt64Ty();
      case ir::Type::kCharacter:
        return builder.getInt32Ty();
      case ir::Type::kBoolean:
        return builder.getInt1Ty();
      case ir::Type::kString:
        return string_type;
    }
    throw std::logic_error{"invalid IR type"};
  };
  std::vector<llvm::Function*> functions;
  for (std::size_t i = 0; i < program.functions.size(); ++i) {
    const auto& input = program.functions[i];
    std::vector<llvm::Type*> parameters;
    for (const auto parameter : input.parameters) {
      parameters.push_back(type(parameter));
    }
    auto* return_type = input.return_type ? type(*input.return_type) : builder.getVoidTy();
    auto* signature = llvm::FunctionType::get(return_type, parameters, false);
    const bool entry = i == program.entry_point;
    functions.push_back(
      llvm::Function::Create(
        signature,
        entry ? llvm::Function::ExternalLinkage : llvm::Function::InternalLinkage,
        entry ? "epp_entry" :"epp_function_" + std::to_string(i), *module
      )
    );
  }
  for (std::size_t i = 0; i < program.functions.size(); ++i) {
    const auto& input = program.functions[i];
    auto* function = functions[i];
    if (input.blocks.empty()) {
      throw std::logic_error{"IR function needs entry block"};
    }
    std::vector<llvm::BasicBlock*> blocks;
    for (std::size_t j = 0; j < input.blocks.size(); ++j) {
      blocks.push_back(llvm::BasicBlock::Create(context, j == 0 ? "entry" : "block", function));
    }
    // 整数演算の検査で分割した後のブロックを、終端とPhiの参照先に使う。
    auto tails = blocks;
    builder.SetInsertPoint(blocks[0]);
    std::vector<llvm::AllocaInst*> variables;
    for (const auto variable : input.variables) {
      variables.push_back(builder.CreateAlloca(type(variable)));
    }
    std::vector<llvm::Value*> values;
    const auto value = [&](ir::ExpressionId id) -> llvm::Value* {
      if (id.index >= values.size() || values[id.index]->getType()->isVoidTy()) {
        throw std::logic_error{"IR must refer to a preceding value"};
      }
      return values[id.index];
    };
    // 値の依存順で命令を作り、所属blockに配置する。終端は最後に付ける。
    for (const auto& expression : input.expressions) {
      builder.SetInsertPoint(tails.at(expression.block));
      auto* result = std::visit([&](const auto& data) -> llvm::Value* {
        using T = std::decay_t<decltype(data)>;
        if constexpr (std::is_same_v<T, ir::IntegerConstant>) {
          return llvm::ConstantInt::getSigned(builder.getInt64Ty(), data.value);
        } else if constexpr (std::is_same_v<T, ir::BooleanConstant>) {
          return builder.getInt1(data.value);
        } else if constexpr (std::is_same_v<T, ir::CharacterConstant>) {
          return builder.getInt32(static_cast<std::uint32_t>(data.value));
        } else if constexpr (std::is_same_v<T, ir::StringConstant>) {
          return CreateStringConstant(*module, string_type, data.value);
        } else if constexpr (std::is_same_v<T, ir::ParameterReference>) {
          if (data.index >= function->arg_size()) {
            throw std::logic_error{"invalid IR parameter"};
          }
          return function->getArg(static_cast<unsigned>(data.index));
        } else if constexpr (std::is_same_v<T, ir::LoadExpression>) {
          return builder.CreateLoad(type(input.variables.at(data.variable)), variables.at(data.variable));
        } else if constexpr (std::is_same_v<T, ir::StoreExpression>) {
          return builder.CreateStore(value(data.value), variables.at(data.variable));
        } else if constexpr (std::is_same_v<T, ir::IntegerBinaryExpression>) {
          return EmitIntegerOperation(builder, data.operation, value(data.left), value(data.right));
        } else if constexpr (std::is_same_v<T, ir::CompareExpression>) {
          auto* left = value(data.left);
          auto* right = value(data.right);
          if (left->getType() == string_type) {
            if (data.operation != ir::Comparison::kEqual && data.operation != ir::Comparison::kNotEqual) {
              throw std::logic_error{"string comparison must use equality or inequality"};
            }
            auto* equal = builder.CreateCall(StringEqual(*module, string_type), {left, right});
            return data.operation == ir::Comparison::kEqual ? equal : builder.CreateNot(equal);
          }
          return builder.CreateICmp(Predicate(data.operation), left, right);
        } else if constexpr (std::is_same_v<T, ir::NotExpression>) {
          return builder.CreateNot(value(data.operand));
        } else if constexpr (std::is_same_v<T, ir::StringLengthExpression>) {
          auto* string = value(data.string);
          auto* length = builder.CreateExtractValue(string, 1);
          auto* signature = llvm::FunctionType::get(
            builder.getInt64Ty(), {builder.getInt64Ty()}, false
          );
          return builder.CreateCall(
            module->getOrInsertFunction("epp_string_length", signature),
            {length}
          );
        } else if constexpr (std::is_same_v<T, ir::StringCharacterExpression>) {
          auto* string = value(data.string);
          auto* characters = builder.CreateExtractValue(string, 0);
          auto* length = builder.CreateExtractValue(string, 1);
          auto* signature = llvm::FunctionType::get(
            builder.getInt32Ty(),
            {builder.getPtrTy(), builder.getInt64Ty(), builder.getInt64Ty()},
            false
          );
          return builder.CreateCall(
            module->getOrInsertFunction("epp_string_character", signature),
            {characters, length, value(data.index)}
          );
        } else if constexpr (std::is_same_v<T, ir::InputExpression>) {
          llvm::IRBuilder<> entry_builder{blocks[0], blocks[0]->begin()};
          auto* data_slot = entry_builder.CreateAlloca(builder.getPtrTy());
          auto* length_slot = entry_builder.CreateAlloca(builder.getInt64Ty());
          auto* signature = llvm::FunctionType::get(
            builder.getVoidTy(), {builder.getPtrTy(), builder.getPtrTy()}, false
          );
          builder.CreateCall(
            module->getOrInsertFunction("epp_input_line", signature),
            {data_slot, length_slot}
          );
          auto* data_ptr = builder.CreateLoad(builder.getPtrTy(), data_slot);
          auto* length = builder.CreateLoad(builder.getInt64Ty(), length_slot);
          auto* result = builder.CreateInsertValue(
            llvm::UndefValue::get(string_type), data_ptr, 0
          );
          return builder.CreateInsertValue(result, length, 1);
        } else if constexpr (std::is_same_v<T, ir::TapeStepExpression>) {
          auto* signature = llvm::FunctionType::get(
            builder.getInt64Ty(), {builder.getInt64Ty(), builder.getInt32Ty()}, false
          );
          return builder.CreateCall(
            module->getOrInsertFunction("epp_tape_step", signature),
            {value(data.value),
              builder.CreateZExt(value(data.right), builder.getInt32Ty())}
          );
        } else if constexpr (std::is_same_v<T, ir::OutputInstruction>) {
          return EmitOutput(*module, builder, data, value(data.value), type(data.type));
        } else if constexpr (std::is_same_v<T, ir::PhiExpression>) {
          auto* phi = builder.CreatePHI(builder.getInt1Ty(), 2);
          phi->addIncoming(value(data.left), tails.at(data.left_block));
          phi->addIncoming(value(data.right), tails.at(data.right_block));
          return phi;
        } else {
          auto* callee = functions.at(data.function_index);
          if (data.arguments.size() != callee->arg_size()) {
            throw std::logic_error{"invalid IR argument count"};
          }
          std::vector<llvm::Value*> arguments;
          for (const auto argument : data.arguments) {
            arguments.push_back(value(argument));
          }
          return builder.CreateCall(callee, arguments, callee->getReturnType()->isVoidTy() ? "" : "call");
        }},
        expression.data
      );
      values.push_back(result);
      tails.at(expression.block) = builder.GetInsertBlock();
    }
    for (std::size_t j = 0; j < input.blocks.size(); ++j) {
      builder.SetInsertPoint(tails[j]);
      std::visit([&](const auto& exit) {
        using T = std::decay_t<decltype(exit)>;
        if constexpr (std::is_same_v<T, ir::Return>) {
          if (exit.value) {
            builder.CreateRet(value(*exit.value));
          } else {
            builder.CreateRetVoid();
          }
        } else if constexpr (std::is_same_v<T, ir::Jump>) {
          builder.CreateBr(blocks.at(exit.target));
        } else if constexpr (std::is_same_v<T, ir::Branch>) {
          builder.CreateCondBr(value(exit.condition), blocks.at(exit.yes), blocks.at(exit.no));
        } else {
          builder.CreateUnreachable();
        }},
        input.blocks[j].exit
      );
    }
  }
  if (options.emit_main) {
    EmitMain(*module, builder, functions[program.entry_point]);
  }

  return module;
}

void VerifyModule(const llvm::Module& module) {
  std::string error;
  llvm::raw_string_ostream output{error};
  if (llvm::verifyModule(module, &output)) {
    output.flush();
    throw std::logic_error{"invalid generated LLVM IR: " + error};
  }
}

} // namespace epp::codegen::detail
