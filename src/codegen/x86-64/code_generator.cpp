#include "code_generator.hpp"
#include "braw_context.hpp"
#include "codegen/operand.hpp"
#include "codegen/x86-64/address.hpp"
#include "codegen/x86-64/immediate.hpp"
#include "codegen/x86-64/instruction.hpp"
#include "codegen/x86-64/label.hpp"
#include "codegen/x86-64/move-resolver/move_resolver.hpp"
#include "codegen/x86-64/olabel.hpp"
#include "codegen/x86-64/register.hpp"
#include "cursor.hpp"
#include "ir/address.hpp"
#include "ir/function.hpp"
#include "ir/instruction.hpp"
#include "ir/instructions/basic.hpp"
#include "ir/instructions/call.hpp"
#include "ir/label.hpp"
#include "ir/operand.hpp"
#include "ir/register.hpp"
#include "ir/value.hpp"
#include "rules.hpp"
#include "type_info.hpp"
#include "utils.hpp"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace CodeGen::x86_64 {

using Operands::Register;

const Register::RegisterGroup SPILL1 = Register::R15;
const Register::RegisterGroup SPILL2 = Register::R14;
const Register::RegisterGroup PRCSPILL1 = Register::XMM15;
const Register::RegisterGroup PRCSPILL2 = Register::XMM14;

template <typename T>
std::shared_ptr<T> cast(const std::shared_ptr<void>& ptr) {
    return std::static_pointer_cast<T>(ptr);
}

void addInstruction(Instruction instr, FunctionContext& ctx) {
    ctx.m_file.m_text.m_instructions.push_back(std::move(instr));
    ctx.m_file.m_text.m_instructions.back().m_irIndex = ctx.m_instructionIndex;
    ctx.m_file.m_text.m_instructions.back().m_irFunctionIndex = ctx.m_functionIndex;
}

File CodeGenerator::generate(const ::File& src, BrawContext& braw) {
    File file;

    for(const Function* f : src.m_externals)
        file.m_text.m_externals.emplace_back(f->m_name);

    size_t idx = 0;
    for(const Function& f : src.m_functions) {
        if(f.m_external) {
            idx++;
            continue;
        }
        initializeRegisters();

        file.m_text.m_globals.push_back({f.m_name});

        RegisterAllocatorResult result = RegisterAllocator::build(f, {Register::RDI,Register::RSI,Register::RDX,Register::RCX,Register::R8,Register::R9,Register::RBX,Register::R10,Register::R11,Register::R12,Register::R13,Register::R14,Register::R15}, {Register::XMM0,Register::XMM1,Register::XMM2,Register::XMM3,Register::XMM4,Register::XMM5,Register::XMM6,Register::XMM7,Register::XMM8,Register::XMM9,Register::XMM10,Register::XMM11,Register::XMM12,Register::XMM13,Register::XMM14,Register::XMM15}, 6, 6);
        FunctionContext ctx{file, braw, result};
        ctx.m_virtualRegisters["%return"] = m_registers.at(Operands::Register::RAX);
        ctx.m_virtualRegisters["%returnF"] = m_registers.at(Operands::Register::XMM0);
        ctx.m_functionIndex = idx++;

        for(auto& retains : f.m_retains) {
            TypeInfo type = retains.second->m_scale > 1 ? Utils::getRawType(retains.second->m_type, braw).value() : retains.second->m_type;
            file.m_data.m_retains.push_back({retains.first.substr(1), std::move(type), retains.second->m_scale});
            ctx.m_virtualRegisters[retains.first] = std::make_shared<Operands::Address>(std::make_shared<Operands::Label>(retains.first.substr(1)), retains.second->m_type);
        }

        generate(f.m_instructions.at(0).get(), ctx); //label

        push(m_registers.at(Operands::Register::RBP), ctx);
        ctx.m_spills = 0;
        move(m_registers.at(Operands::Register::RBP), m_registers.at(Operands::Register::RSP), ctx);

        std::unordered_set<Operands::Register::RegisterGroup> savedRegisters;

        int64_t spills = 0;
        for(auto block : result.m_propagated.blocks) {
            for(auto range : block->m_rangeVector) {
                if(ctx.m_virtualRegisters.contains(range->m_id))
                    continue;
                if(!range->m_isPointedOrDereferenced && result.m_registers.contains(range->m_id)) {
                    ctx.m_virtualRegisters[range->m_id] = range->m_registerType == RegisterType::Struct ? cast<Operand>(std::make_shared<Operands::Address>(m_registers.at(result.m_registers.at(range->m_id)), -range->m_typeInfo.m_size, range->m_typeInfo)) : m_registers.at(result.m_registers.at(range->m_id))->clone();
                    ctx.m_virtualRegisters[range->m_id]->m_typeInfo = range->m_typeInfo;
                    ctx.m_virtualRegisters[range->m_id]->m_scaleSize = range->m_scale;
                    Operands::Register::RegisterGroup group = result.m_registers.at(range->m_id);
                    if(group == Operands::Register::RBX || group == Operands::Register::R12 || group == Operands::Register::R13 || group == Operands::Register::R14 || group == Operands::Register::R15)
                        savedRegisters.insert(group);
                }
                else {
                    TypeInfo type = range->m_scale > 1 ? Utils::getRawType(range->m_typeInfo, ctx.m_brawCtx).value() : range->m_typeInfo;
                    spills += range->m_scale > 1 ? type.m_size * range->m_scale : range->m_typeInfo.m_size;
                    ctx.m_virtualRegisters[range->m_id] = std::make_shared<Operands::Address>(m_registers.at(Operands::Register::RBP), -spills, type);
                    ctx.m_virtualRegisters[range->m_id]->m_scaleSize = range->m_scale;
                    ctx.m_virtualRegisters[range->m_id]->m_typeInfo = range->m_typeInfo;
                    for(auto& arg : f.m_args) {
                        if(arg->m_id == range->m_id) {
                            m_registers.at(result.m_registers.at(range->m_id))->m_typeInfo = range->m_typeInfo;
                            move(ctx.m_virtualRegisters.at(range->m_id), m_registers.at(result.m_registers.at(range->m_id)), ctx);
                            break;
                        }
                    }
                }
            }
        }
        spills += spills % 16; // 16 byte alignment
        ctx.m_spills = spills;

        for(auto group : savedRegisters)
            ctx.m_savedRegisters.push_back(m_registers.at(group));

        for(auto reg : ctx.m_savedRegisters)
            push(reg, ctx);

        if(spills > 0)
            sub(m_registers.at(Operands::Register::RSP), std::make_shared<Operands::Immediate>(spills, ctx.m_brawCtx.getTypeInfo(INT_T).value()), ctx);

        //skip label
        for(size_t i = 1; i < f.m_instructions.size(); ++i) {
            auto& in = f.m_instructions[i];
            ctx.m_instructionIndex = i;
            generate(in.get(), ctx);
        }
    }

    // coalesce useless moves
    std::erase_if(file.m_text.m_instructions, [](const Instruction& i) {
        if((i.m_opcode == Mov || i.m_opcode == Movss || i.m_opcode == Movsd) && MoveResolver::operandEquals(i.m_operands.at(0), i.m_operands.at(1)))
            return true;
        return false;
    });

    return file;
}

void CodeGenerator::generate(const ::Instruction* instr, FunctionContext& ctx) {
    switch(instr->m_type) {
        case ::Instruction::Label: {
            Instruction i; i.m_opcode = LabelOp;
            i.m_operands.push_back(std::make_shared<Operands::Label>(((const ::Label*)instr)->m_id));
            addInstruction(i, ctx);
            break;
        }
        case ::Instruction::Move: {
            auto bin = (const ::BasicInstruction*)instr;
            move(convertOperand(bin->m_o1, ctx), convertOperand(bin->m_o2, ctx), ctx); 
            break;
        }
        case ::Instruction::Add: {
            auto bin = (const ::BasicInstruction*)instr;
            add(convertOperand(bin->m_o1, ctx), convertOperand(bin->m_o2, ctx), ctx); 
            break;
        }
        case ::Instruction::Subtract: {
            auto bin = (const ::BasicInstruction*)instr;
            sub(convertOperand(bin->m_o1, ctx), convertOperand(bin->m_o2, ctx), ctx); 
            break;
        }
        case ::Instruction::Multiply: {
            auto bin = (const ::BasicInstruction*)instr;
            mul(convertOperand(bin->m_o1, ctx), convertOperand(bin->m_o2, ctx), ctx); 
            break;
        }
        case ::Instruction::Divide: {
            auto bin = (const ::BasicInstruction*)instr;
            div(convertOperand(bin->m_o1, ctx), convertOperand(bin->m_o2, ctx), ctx); 
            break;
        }
        case ::Instruction::Modulo: {
            auto bin = (const ::BasicInstruction*)instr;
            mod(convertOperand(bin->m_o1, ctx), convertOperand(bin->m_o2, ctx), ctx); 
            break;
        }
        case ::Instruction::CompareEquals: {
            auto bin = (const ::BasicInstruction*)instr;
            compareAndStore(cast<Operands::Register>(convertOperand(bin->m_o1, ctx)), convertOperand(bin->m_o2, ctx), cast<Operands::Register>(convertOperand(bin->m_o3, ctx)), Sete, ctx); 
            break;
        }
        case ::Instruction::CompareNotEquals: {
            auto bin = (const ::BasicInstruction*)instr;
            compareAndStore(cast<Operands::Register>(convertOperand(bin->m_o1, ctx)), convertOperand(bin->m_o2, ctx), cast<Operands::Register>(convertOperand(bin->m_o3, ctx)), Setne, ctx); 
            break;
        }
        case ::Instruction::CompareGreater: {
            auto bin = (const ::BasicInstruction*)instr;
            compareAndStore(cast<Operands::Register>(convertOperand(bin->m_o1, ctx)), convertOperand(bin->m_o2, ctx), cast<Operands::Register>(convertOperand(bin->m_o3, ctx)), Setg, ctx); 
            break;
        }
        case ::Instruction::CompareLess: {
            auto bin = (const ::BasicInstruction*)instr;
            compareAndStore(cast<Operands::Register>(convertOperand(bin->m_o1, ctx)), convertOperand(bin->m_o2, ctx), cast<Operands::Register>(convertOperand(bin->m_o3, ctx)), Setl, ctx); 
            break;
        }
        case ::Instruction::CompareGreaterEquals: {
            auto bin = (const ::BasicInstruction*)instr;
            compareAndStore(cast<Operands::Register>(convertOperand(bin->m_o1, ctx)), convertOperand(bin->m_o2, ctx), cast<Operands::Register>(convertOperand(bin->m_o3, ctx)), Setge, ctx); 
            break;
        }
        case ::Instruction::CompareLessEquals: {
            auto bin = (const ::BasicInstruction*)instr;
            compareAndStore(cast<Operands::Register>(convertOperand(bin->m_o1, ctx)), convertOperand(bin->m_o2, ctx), cast<Operands::Register>(convertOperand(bin->m_o3, ctx)), Setle, ctx); 
            break;
        }
        case ::Instruction::JumpFalse: {
            auto bin = (const ::BasicInstruction*)instr;
            compareAndJump(cast<Operands::Register>(convertOperand(bin->m_o1, ctx)), std::make_shared<Operands::Immediate>(0, ctx.m_brawCtx.getTypeInfo(INT_T).value()), cast<Operands::Label>(convertOperand(bin->m_o2, ctx)), Je, ctx);
            break;
        }
        case ::Instruction::JumpTrue: {
            auto bin = (const ::BasicInstruction*)instr;
            compareAndJump(cast<Operands::Register>(convertOperand(bin->m_o1, ctx)), std::make_shared<Operands::Immediate>(0, ctx.m_brawCtx.getTypeInfo(INT_T).value()), cast<Operands::Label>(convertOperand(bin->m_o2, ctx)), Jne, ctx);
            break;
        }
        case ::Instruction::And: {
            auto bin = (const ::BasicInstruction*)instr;
            Instruction in;
            in.m_opcode = And;
            in.addOperand(convertOperand(bin->m_o1, ctx));
            in.addOperand(convertOperand(bin->m_o2, ctx));
            addInstruction(in, ctx);
            break;
        }
        case ::Instruction::Or: {
            auto bin = (const ::BasicInstruction*)instr;
            Instruction in;
            in.m_opcode = Or;
            in.addOperand(convertOperand(bin->m_o1, ctx));
            in.addOperand(convertOperand(bin->m_o2, ctx));
            addInstruction(in, ctx);
            break;
        }
        case ::Instruction::Xor: {
            auto bin = (const ::BasicInstruction*)instr;
            Instruction in;
            in.m_opcode = Xor;
            in.addOperand(convertOperand(bin->m_o1, ctx));
            in.addOperand(convertOperand(bin->m_o2, ctx));
            addInstruction(in, ctx);
            break;
        }
        case ::Instruction::LogicalNot: {
            auto bin = (const ::BasicInstruction*)instr;
            Instruction in, sete;
            in.m_opcode = Test;
            auto op2 = convertOperand(bin->m_o2, ctx);
            if(op2->m_type == Operand::Type::Immediate) {
                move(m_registers.at(SPILL1), op2, ctx);
                op2 = m_registers.at(SPILL1);
            }
            op2 = op2->m_type == Operand::Type::Address ? memoryValueToRegister(cast<Operands::Address>(op2), ctx) : op2;
            in.addOperand(op2);
            in.addOperand(op2);
            addInstruction(in, ctx);
            sete.m_opcode = Sete;
            sete.addOperand(convertOperand(bin->m_o1, ctx));
            addInstruction(sete, ctx);
            break;
        }
        case ::Instruction::Jump: {
            auto bin = (const ::BasicInstruction*)instr;
            Instruction in;
            in.m_opcode = Jmp;
            in.addOperand(cast<Operands::Label>(convertOperand(bin->m_o1, ctx)));
            addInstruction(in, ctx);
            break;
        }
        case ::Instruction::Call: {
            auto callIn = (const ::CallInstruction*)instr;
            auto optRet = callIn->m_optReturn ? cast<Operands::Register>(convertOperand(callIn->m_optReturn, ctx)) : nullptr;
            if(optRet)
                optRet->m_typeInfo = callIn->m_optReturn->m_type;

            if(!callIn->m_returnType.m_builtin) {
                auto addr = cast<Operands::Address>(convertOperand(callIn->m_optReturn, ctx)->clone());
                addr->m_offset += callIn->m_returnType.m_size;
                memoryAddressToRegister(addr, m_registers.at(Operands::Register::RDI), ctx);
            }

            call(std::make_shared<Operands::Label>(callIn->m_id), callIn->m_returnType.m_builtin ? optRet : nullptr, callIn->m_parameters, callIn->m_returnType.m_builtin ? 0 : 1, ctx);
            break;
        }
        case ::Instruction::Return:
            return ret(ctx);
        case ::Instruction::Copy: {
            auto bin = (const ::BasicInstruction*)instr;
            auto addr1 = convertOperand(bin->m_o1, ctx);
            auto addr2 = convertOperand(bin->m_o2, ctx);
            if(addr2->m_type == Operand::Type::Register)
                copyAddressToAddressPointer(addr1, addr2, addr1->m_typeInfo.m_size, ctx);
            else
                copyAddressToAddress(addr1, addr2, addr1->m_typeInfo.m_size, ctx);
            break;
        }
        case ::Instruction::Point: {
            auto bin = (const ::BasicInstruction*)instr;
            auto addr = cast<Operands::Address>(convertOperand(bin->m_o1, ctx));
            auto orig = convertOperand(bin->m_o2, ctx);
            auto spill = memoryAddressToRegister(orig->m_type == Operand::Type::Register ? std::make_shared<Operands::Address>(orig, 0, orig->m_typeInfo) : cast<Operands::Address>(orig), ctx)->clone();
            move(addr,spill, ctx);
            break;
        }
        case ::Instruction::Dereference: {
            auto bin = (const ::BasicInstruction*)instr;
            auto target = convertOperand(bin->m_o1, ctx);
            auto addr = cast<Operands::Address>(convertOperand(bin->m_o2, ctx)->clone());
            if(bin->m_o1.index() == 1 && !std::get<std::shared_ptr<::Register>>(bin->m_o1)->m_type.m_builtin) {
                copyAddressToAddressPointer(cast<Operands::Address>(target), addr, std::get<std::shared_ptr<::Register>>(bin->m_o1)->m_type.m_size, ctx);
                break;
            }
            move(m_registers.at(SPILL1), addr, ctx);
            auto deref = std::make_shared<Operands::Address>(m_registers.at(SPILL1), 0, Utils::getRawType(m_registers.at(SPILL1)->m_typeInfo, ctx.m_brawCtx).value());
            move(target, deref, ctx);
            break;
        }
        case ::Instruction::PartialDereference: {
            auto bin = (const ::BasicInstruction*)instr;
            auto target = convertOperand(bin->m_o1, ctx);
            auto addr = cast<Operands::Address>(convertOperand(bin->m_o2, ctx)->clone());
            move(target, addr, ctx);
            break;
        }
        case ::Instruction::Upsize: {
            auto bin = (const ::BasicInstruction*)instr;
            auto o1 = convertOperand(bin->m_o1, ctx);
            auto o2 = convertOperand(bin->m_o2, ctx);
            if(o2->m_typeInfo.m_name == INT_T || o2->m_typeInfo.m_name == CHAR_T) {
                if(Rules::isPtr(o1->m_typeInfo.m_name) || o1->m_typeInfo.m_name == LONG_T || o1->m_typeInfo.m_name == INT_T) {
                    Instruction in; in.m_opcode = o2->m_typeInfo.m_name == CHAR_T ? Movsx : Movsxd;
                    auto maybeReg = m_registers.at(SPILL1)->clone();
                    maybeReg->m_typeInfo = o1->m_typeInfo;
                    in.addOperand(o1->m_type != Operand::Type::Address ? o1 : maybeReg);
                    if(o2->m_type == Operand::Type::Immediate) {
                        move(m_registers.at(SPILL2), o2, ctx);
                        o2 = m_registers.at(SPILL2);
                    }
                    in.addOperand(o2);
                    addInstruction(in, ctx);
                    if(o1->m_type == Operand::Type::Address)
                        move(o1, in.m_operands[0], ctx);
                }
            }
            break;
        }
        case ::Instruction::Downsize: {
            auto bin = (const ::BasicInstruction*)instr;
            auto o1 = convertOperand(bin->m_o1, ctx);
            auto o2 = convertOperand(bin->m_o2, ctx);
            if(Rules::isPtr(o2->m_typeInfo.m_name) || o2->m_typeInfo.m_name == LONG_T || o2->m_typeInfo.m_name == INT_T) {
                if(o2->m_type == Operand::Type::Immediate) {
                    move(m_registers.at(SPILL2), o2, ctx);
                    o2 = m_registers.at(SPILL2);
                }
                std::shared_ptr<Register> intermediate = o2->m_type == Operand::Type::Register ? cast<Operands::Register>(o2->clone()) : memoryValueToRegister(cast<Operands::Address>(o2), ctx);
                intermediate->m_typeInfo = o1->m_typeInfo;
                move(o1, intermediate, ctx);
            }
        }
        default: break;
    }
}

void CodeGenerator::move(std::shared_ptr<Operand> target, std::shared_ptr<Operand> source, FunctionContext& ctx) {
    if(source->m_scaleSize > 1)
        source = memoryAddressToRegister(cast<Operands::Address>(source), ctx);
    
    Instruction in;
    if(bothAddress(target, source)) source = memoryValueToRegister(cast<Operands::Address>(source), ctx);
    
    if(isFloat(source)) in.m_opcode = Movss;
    else if(isDouble(source)) in.m_opcode = Movsd;
    else in.m_opcode = Mov;
    
    target->m_typeInfo = source->m_typeInfo;
    target->m_scaleSize = source->m_scaleSize;
    in.addOperand(target);
    in.addOperand(source);
    addInstruction(std::move(in), ctx);
}

void CodeGenerator::add(std::shared_ptr<Operand> target, std::shared_ptr<Operand> source, FunctionContext& ctx) {
    Instruction in;
    if(bothAddress(target, source)) source = memoryValueToRegister(std::static_pointer_cast<Operands::Address>(source), ctx);
    in.m_opcode = isFloat(source) ? Addss : isDouble(source) ? Addsd : Add;
    in.addOperand(target);
    in.addOperand(source);
    addInstruction(std::move(in), ctx);
}

void CodeGenerator::sub(std::shared_ptr<Operand> target, std::shared_ptr<Operand> source, FunctionContext& ctx) {
    Instruction in;
    if(bothAddress(target, source)) source = memoryValueToRegister(std::static_pointer_cast<Operands::Address>(source), ctx);
    in.m_opcode = isFloat(source) ? Subss : isDouble(source) ? Subsd : Sub;
    in.addOperand(target);
    in.addOperand(source);
    addInstruction(std::move(in), ctx);
}

void CodeGenerator::mul(std::shared_ptr<Operand> target, std::shared_ptr<Operand> source, FunctionContext& ctx) {
    Instruction in;
    if(bothAddress(target, source)) source = memoryValueToRegister(std::static_pointer_cast<Operands::Address>(source), ctx);
    in.m_opcode = isFloat(source) ? Mulss : isDouble(source) ? Mulsd : isUnsigned(source) ? Mul : Imul;
    in.addOperand(target);
    in.addOperand(source);
    addInstruction(std::move(in), ctx);
}

void CodeGenerator::div(std::shared_ptr<Operand> target, std::shared_ptr<Operand> source, FunctionContext& ctx) {
    if(source->m_typeInfo.m_name == INT_T || source->m_typeInfo.m_name == LONG_T) {
        std::vector<std::shared_ptr<Operands::Register>> saveStack;
        if(isRegisterAlive(Register::RAX, ctx)) saveStack.push_back(m_registers.at(Register::RAX));
        if(isRegisterAlive(Register::RDX, ctx)) saveStack.push_back(m_registers.at(Register::RDX));
        for(auto reg : saveStack)
            push(reg, ctx);
        move(m_registers.at(Register::RAX),target, ctx);
        Instruction in; in.m_opcode = source->m_typeInfo.m_name == INT_T ? Cdq : Cqo; addInstruction(in, ctx);
        if(source->m_type != Operand::Type::Register) {
            move(m_registers.at(SPILL1), source, ctx);
            source = m_registers.at(SPILL1);
        }

        in.m_operands.clear(); in.m_opcode = isUnsigned(source) ? Div : Idiv; in.addOperand(source); addInstruction(in, ctx);
        move(target, m_registers.at(Register::RAX), ctx);
        std::reverse(saveStack.begin(), saveStack.end());
        for(auto reg : saveStack)
            pop(reg, ctx);
        return;
    }
    Instruction in;
    if(bothAddress(target, source)) source = memoryValueToRegister(std::static_pointer_cast<Operands::Address>(source), ctx);
    in.m_opcode = isFloat(source) ? Divss : Divsd;
    in.addOperand(target);
    in.addOperand(source);
    addInstruction(std::move(in), ctx);
}

void CodeGenerator::mod(std::shared_ptr<Operand> target, std::shared_ptr<Operand> source, FunctionContext& ctx) {
    std::vector<std::shared_ptr<Operands::Register>> saveStack;
    if(isRegisterAlive(Register::RAX, ctx)) saveStack.push_back(m_registers.at(Register::RAX));
    if(isRegisterAlive(Register::RDX, ctx)) saveStack.push_back(m_registers.at(Register::RDX));
    for(auto reg : saveStack)
        push(reg, ctx);
    move(m_registers.at(Register::RAX),target, ctx);
    Instruction in; in.m_opcode = source->m_typeInfo.m_name == INT_T ? Cdq : Cqo; addInstruction(in, ctx);
    if(source->m_type != Operand::Type::Register) {
        move(m_registers.at(SPILL1), source, ctx);
        source = m_registers.at(SPILL1);
    }

    in.m_operands.clear(); in.m_opcode = isUnsigned(source) ? Div : Idiv; in.addOperand(source); addInstruction(in, ctx);
    auto rdxClone = m_registers.at(Register::RDX);
    rdxClone->m_typeInfo = m_registers.at(Register::RAX)->m_typeInfo;
    move(target, rdxClone, ctx);
    std::reverse(saveStack.begin(), saveStack.end());
    for(auto reg : saveStack)
        pop(reg, ctx);
}

void CodeGenerator::call(std::shared_ptr<Operands::Label> label, std::shared_ptr<Operands::Register> optReturn, const std::vector<::Operand>& args, size_t skipArgs, FunctionContext& ctx) {
    static const std::unordered_set<Register::RegisterGroup> callerSaved = {Register::RAX,Register::RCX,Register::RDX,Register::RSI,Register::RDI,Register::R8,Register::R9,Register::R10,Register::R11,Register::XMM0,Register::XMM1,Register::XMM2,Register::XMM3,Register::XMM4,Register::XMM5,Register::XMM6,Register::XMM7};
    std::vector<std::shared_ptr<Operands::Register>> saveStack;

    size_t blockIndex = ctx.m_allocatorResult.m_propagated.blockForInstruction.at(ctx.m_instructionIndex);
    for(auto range : ctx.m_allocatorResult.m_propagated.blocks.at(blockIndex)->m_rangeVector) {
        if(!(range->m_range.first <= ctx.m_instructionIndex && ctx.m_instructionIndex <= range->m_range.second)/*  || range->m_isAssignedFirst TODO: figure this out */)
            continue;

        std::shared_ptr<Operand> arg = ctx.m_virtualRegisters.at(range->m_id);

        if(arg->m_type != Operand::Type::Register || !callerSaved.contains(cast<Operands::Register>(arg)->m_group) || (optReturn &&cast<Operands::Register>(arg)->m_group == optReturn->m_group))
            continue;

        auto reg = cast<Operands::Register>(arg);

        saveStack.push_back(reg);
        push(reg, ctx);
    }

    std::array<Operands::Register::RegisterGroup, 6> parameterRegisters = {Operands::Register::RDI, Operands::Register::RSI, Operands::Register::RDX, Operands::Register::RCX, Operands::Register::R8, Operands::Register::R9};
    Cursor<std::array<Operands::Register::RegisterGroup, 6>::iterator> cursor(parameterRegisters.begin(), parameterRegisters.end());
    std::array<Operands::Register::RegisterGroup, 6> floatParameterRegisters = {Operands::Register::XMM0, Operands::Register::XMM1, Operands::Register::XMM2, Operands::Register::XMM3, Operands::Register::XMM4, Operands::Register::XMM5};
    Cursor<std::array<Operands::Register::RegisterGroup, 6>::iterator> floatCursor(floatParameterRegisters.begin(), floatParameterRegisters.end());

    if(skipArgs > 0)
        cursor.next(skipArgs);

    size_t spilledBeg = ctx.m_spills;
    size_t beg = ctx.m_file.m_text.m_instructions.size();
    std::unordered_set<size_t> ignore;
    for(auto& arg : args) {
        auto op = convertOperand(arg, ctx);
        
        if((isFloat(op) && !floatCursor.hasNext()) || (isDouble(op) && !floatCursor.hasNext()) || ((op->m_typeInfo.m_name == INT_T || op->m_typeInfo.m_name == LONG_T || Rules::isPtr(op->m_typeInfo.m_name)) && !cursor.hasNext())) {
            push(op, ctx);
            continue;
        }

        if(isFloat(op)) {
            Operands::Register::RegisterGroup reg = floatCursor.get().next().value();
            move(m_registers.at(reg), op, ctx);
        } else {
            auto reg = cursor.get().next().value();
            if(arg.index() == 1 && std::get<1>(arg)->m_registerType == RegisterType::Struct) {
                size_t begg = ctx.m_file.m_text.m_instructions.size();
                op = copyAddressToNew(cast<Operands::Address>(op), std::get<1>(arg)->m_type.m_size, ctx);
                for(; begg < ctx.m_file.m_text.m_instructions.size(); ++begg) {
                    ignore.insert(begg - beg);
                }
            }

            if(!op->m_typeInfo.m_builtin && op->m_type == Operand::Type::Address) {
                auto addr = cast<Operands::Address>(op->clone());
                addr->m_offset += std::get<1>(arg)->m_type.m_size;
                memoryAddressToRegister(addr, m_registers.at(reg), ctx);
            }
            else {
                if(op->m_type == Operand::Type::Address && cast<Operands::Address>(op)->m_base->m_type == Operand::Type::Label) {
                    auto spill = memoryAddressToRegister(op->m_type == Operand::Type::Register ? std::make_shared<Operands::Address>(op, 0, op->m_typeInfo) : cast<Operands::Address>(op), ctx)->clone();
                    move(m_registers.at(reg),spill, ctx);
                }
                else if(op->m_scaleSize <= 1)
                    move(m_registers.at(reg), op, ctx);
                else
                    memoryAddressToRegister(cast<Operands::Address>(op), m_registers.at(reg), ctx);
            }
        }
    }
    size_t end = ctx.m_file.m_text.m_instructions.size() - 1;
    size_t spilled = ctx.m_spills - spilledBeg;

    std::vector<Instruction> from(ctx.m_file.m_text.m_instructions.begin() + beg, ctx.m_file.m_text.m_instructions.begin() + end + 1);
    std::vector<Instruction> result = MoveResolver::resolve(from, ignore, *this, ctx);
    ctx.m_file.m_text.m_instructions.erase(ctx.m_file.m_text.m_instructions.begin() + beg, ctx.m_file.m_text.m_instructions.begin() + end + 1);
    ctx.m_file.m_text.m_instructions.insert(ctx.m_file.m_text.m_instructions.begin() + beg, result.begin(), result.end());

    Instruction i;
    i.m_opcode = Call;
    i.addOperand(label);
    addInstruction(i, ctx);

    if(spilled > 0) {
        add(m_registers.at(Operands::Register::RSP), std::make_shared<Operands::Immediate>(spilled, ctx.m_brawCtx.getTypeInfo(INT_T).value()), ctx);
        ctx.m_spills -= spilled;
    }

    std::reverse(saveStack.begin(), saveStack.end());
    for(auto reg : saveStack)
        pop(reg, ctx);

    if(optReturn) {
        auto retReg = isFloat(optReturn) || isDouble(optReturn) ? cast<Operands::Register>(m_registers.at(Operands::Register::XMM0)->clone()) : cast<Operands::Register>(m_registers.at(Operands::Register::RAX)->clone());
        retReg->m_typeInfo = optReturn->m_typeInfo;
        retReg->m_registerType = optReturn->m_registerType;
        move(optReturn, retReg, ctx);
    }
}

void CodeGenerator::ret(FunctionContext& ctx) {
    size_t spillP = ctx.m_spills;   
    std::reverse(ctx.m_savedRegisters.begin(), ctx.m_savedRegisters.end());
    for(auto reg : ctx.m_savedRegisters)
        pop(reg, ctx);

    if(ctx.m_spills > 0)
        add(m_registers.at(Operands::Register::RSP), std::make_shared<Operands::Immediate>(ctx.m_spills, ctx.m_brawCtx.getTypeInfo(INT_T).value()), ctx);
    
    pop(m_registers.at(Operands::Register::RBP), ctx);
    Instruction i;
    i.m_opcode = Ret;
    addInstruction(i, ctx);
    ctx.m_spills = spillP;
}

void CodeGenerator::push(std::shared_ptr<Operand> target, FunctionContext& ctx) {
    if(target->m_type == Operand::Type::Immediate) {
        ctx.m_spills += 8;
        Instruction i;
        i.m_opcode = Push;
        i.addOperand(target);
        addInstruction(i, ctx);
        return;
    }
    else if(isRegister(target)) {
        auto reg = cast<Operands::Register>(target);
        Instruction i;

        if(!isFloat(reg) && !isDouble(reg)) {
            ctx.m_spills += 8;
            i.m_opcode = Push;
            auto clone = target->clone();
            // TODO find a way to force qword
            i.addOperand(clone);
            addInstruction(i, ctx);
            return;
        }

        sub(m_registers.at(Operands::Register::RSP), std::make_shared<Operands::Immediate>((int)reg->m_typeInfo.m_size, ctx.m_brawCtx.getTypeInfo(INT_T).value()), ctx);
        ctx.m_spills += target->m_typeInfo.m_size;
        move(std::make_shared<Operands::Address>(m_registers.at(Operands::Register::RSP), -ctx.m_spills, TypeInfo{}), reg, ctx);
        return;
    }

    auto addrReg = memoryAddressToRegister(cast<Operands::Address>(target), ctx);
    Instruction i;
    i.m_opcode = Push;
    i.addOperand(addrReg);
    addInstruction(i, ctx);
}

void CodeGenerator::pop(std::shared_ptr<Operands::Register> target, FunctionContext& ctx) {
    if(isFloat(target) || isDouble(target)) {
        move(target, std::make_shared<Operands::Address>(m_registers.at(Operands::Register::RSP), -ctx.m_spills, target->m_typeInfo), ctx);
        add(m_registers.at(Operands::Register::RSP), std::make_shared<Operands::Immediate>((int)target->m_typeInfo.m_size, ctx.m_brawCtx.getTypeInfo(INT_T).value()), ctx);
        ctx.m_spills -= target->m_typeInfo.m_size;
        return;
    }
    Instruction i;
    i.m_opcode = Pop;
    auto clone = target->clone();
    i.addOperand(clone);
    addInstruction(i, ctx);
    ctx.m_spills -= 8;
}


std::shared_ptr<Operands::Register> CodeGenerator::memoryValueToRegister(std::shared_ptr<Operands::Address> address, FunctionContext& ctx) {
    auto reg = isFloat(address) || isDouble(address) ? m_registers.at(PRCSPILL1) : m_registers.at(SPILL1);
    move(reg, address, ctx);
    return reg;
}

std::shared_ptr<Operands::Register> CodeGenerator::memoryAddressToRegister(std::shared_ptr<Operands::Address> address, FunctionContext& ctx) {
    auto reg = /* isFloat(address) || isDouble(address) ? m_registers.at(PRCSPILL1) : */ m_registers.at(SPILL1);
    return memoryAddressToRegister(address, reg, ctx);
}


std::shared_ptr<Operands::Register> CodeGenerator::memoryAddressToRegister(std::shared_ptr<Operands::Address> address, std::shared_ptr<Operands::Register> reg, FunctionContext& ctx) {
    Instruction i;
    i.m_opcode = Lea;
    reg->m_typeInfo = Utils::makePointer(address->m_typeInfo);
    // TODO find a way to force qword
    i.addOperand(reg);
    i.addOperand(address);
    addInstruction(i, ctx);
    return reg;
}

void CodeGenerator::compareAndStore(std::shared_ptr<Operand> opp, std::shared_ptr<Operand> op, std::shared_ptr<Operands::Register> store, Opcode setOpcode, FunctionContext& ctx) {
    if(opp->m_type == Operand::Type::Immediate || opp->m_type == Operand::Type::Address) {
        auto target = isDouble(opp) || isFloat(opp) ? m_registers.at(PRCSPILL1) : m_registers.at(SPILL1);
        move(target, opp, ctx);
        opp = target;
    }
    if(op->m_type == Operand::Type::Immediate || op->m_type == Operand::Type::Address) {
        auto target = isDouble(op) || isFloat(op) ? m_registers.at(PRCSPILL2) : m_registers.at(SPILL2);
        move(target, op, ctx);
        op = target;
    }
    Instruction cmp, set;
    cmp.m_opcode = isFloat(op) ? Ucomiss : isDouble(op) ? Ucomisd : Cmp;
    cmp.addOperand(opp);
    cmp.addOperand(op);
    addInstruction(cmp, ctx);
    set.m_opcode = setOpcode;
    m_registers.at(Operands::Register::RAX)->m_typeInfo = ctx.m_brawCtx.getTypeInfo(BOOL_T).value();
    set.addOperand(m_registers.at(Operands::Register::RAX));
    addInstruction(set, ctx);
    move(store, m_registers.at(Operands::Register::RAX), ctx);
}

void CodeGenerator::compareAndJump(std::shared_ptr<Operand> opp, std::shared_ptr<Operand> op, std::shared_ptr<Operands::Label> label, Opcode jumpOpcode, FunctionContext& ctx) {
    if(opp->m_type == Operand::Type::Immediate) {
        move(m_registers.at(SPILL1), opp, ctx);
        opp = m_registers.at(SPILL1);
    }
    Instruction cmp, jmp;
    cmp.m_opcode = Cmp;
    cmp.addOperand(opp);
    cmp.addOperand(op);
    addInstruction(cmp, ctx);
    jmp.m_opcode = jumpOpcode;
    jmp.addOperand(label);
    addInstruction(jmp, ctx);
}

std::shared_ptr<Operands::Address> CodeGenerator::copyAddressToNew(std::shared_ptr<Operands::Address> address, size_t size, FunctionContext& ctx) {
    sub(m_registers.at(Operands::Register::RSP), std::make_shared<Operands::Immediate>((int)size, ctx.m_brawCtx.getTypeInfo(INT_T).value()), ctx);
    auto target = std::make_shared<Operands::Address>(m_registers.at(Operands::Register::RBP), -ctx.m_spills, address->m_typeInfo);
    ctx.m_spills += size;
    copyAddressToAddress(target, address, size, ctx);
    return target;
}

void CodeGenerator::copyAddressToAddressPointer(std::shared_ptr<Operand> target, std::shared_ptr<Operand> source, size_t size, FunctionContext& ctx) {
    std::vector<std::shared_ptr<Operands::Register>> save;

    if(isRegisterAlive(Register::RDI, ctx))
        save.push_back(m_registers.at(Register::RDI));
    if(isRegisterAlive(Register::RSI, ctx))
        save.push_back(m_registers.at(Register::RSI));
    if(isRegisterAlive(Register::RCX, ctx))
        save.push_back(m_registers.at(Register::RCX));

    for(auto reg : save)
        push(reg, ctx);

    size_t remainder = size % 8;
    size_t beg = ctx.m_file.m_text.m_instructions.size();

    Instruction lea, mov;
    mov.m_opcode = Mov;
    lea.m_opcode = Lea;
    // TODO find a way to force qword
    lea.addOperand(m_registers.at(Register::RDI));
    lea.addOperand(target);
    m_registers.at(Register::RDI)->m_typeInfo = Utils::makePointer(target->m_typeInfo);
    addInstruction(lea, ctx);
    mov.addOperand(m_registers.at(Register::RSI));
    mov.addOperand(source);
    m_registers.at(Register::RSI)->m_typeInfo = source->m_typeInfo;
    addInstruction(mov, ctx);

    Instruction movs;
    if((size - remainder) / 8 > 0) {
        move(m_registers.at(Register::RCX), std::make_shared<Operands::Immediate>((size - remainder) / 8, ctx.m_brawCtx.getTypeInfo(INT_T).value()), ctx);

        auto movsq = Movsq;
        movs.m_opcode = movsq;
        movs.m_prefix = Rep;
        addInstruction(movs, ctx);
    }

    if(remainder > 0) {
        move(m_registers.at(Register::RCX), std::make_shared<Operands::Immediate>(remainder, ctx.m_brawCtx.getTypeInfo(INT_T).value()), ctx);
        auto movsb = Movsb;
        movs.m_prefix = Rep;
        movs.m_opcode = movsb;
        addInstruction(movs, ctx);
    }
    size_t end = ctx.m_file.m_text.m_instructions.size() - 1;
    std::vector<Instruction> from(ctx.m_file.m_text.m_instructions.begin() + beg, ctx.m_file.m_text.m_instructions.begin() + end + 1);
    std::vector<Instruction> result = MoveResolver::resolve(from, {}, *this, ctx);
    ctx.m_file.m_text.m_instructions.erase(ctx.m_file.m_text.m_instructions.begin() + beg, ctx.m_file.m_text.m_instructions.begin() + end + 1);
    ctx.m_file.m_text.m_instructions.insert(ctx.m_file.m_text.m_instructions.begin() + beg, result.begin(), result.end());

    std::reverse(save.begin(), save.end());
    for(auto reg : save)
        pop(reg, ctx);
}

void CodeGenerator::copyAddressToAddress(std::shared_ptr<Operand> target, std::shared_ptr<Operand> source, size_t size, FunctionContext& ctx) {
    std::vector<std::shared_ptr<Operands::Register>> save;

    if(isRegisterAlive(Register::RDI, ctx))
        save.push_back(m_registers.at(Register::RDI));
    if(isRegisterAlive(Register::RSI, ctx))
        save.push_back(m_registers.at(Register::RSI));
    if(isRegisterAlive(Register::RCX, ctx))
        save.push_back(m_registers.at(Register::RCX));

    for(auto reg : save)
        push(reg, ctx);

    size_t remainder = size % 8;
    size_t beg = ctx.m_file.m_text.m_instructions.size();

    Instruction lea;
    lea.m_opcode = Lea;
    // TODO find a way to force qword
    lea.addOperand(m_registers.at(Register::RDI));
    lea.addOperand(target);
    m_registers.at(Register::RDI)->m_typeInfo = Utils::makePointer(target->m_typeInfo);
    addInstruction(lea, ctx);
    lea.m_operands.clear();
    lea.addOperand(m_registers.at(Register::RSI));
    lea.addOperand(source);
    m_registers.at(Register::RSI)->m_typeInfo = Utils::makePointer(source->m_typeInfo);
    addInstruction(lea, ctx);

    Instruction movs;
    if((size - remainder) / 8 > 0) {
        move(m_registers.at(Register::RCX), std::make_shared<Operands::Immediate>((size - remainder) / 8, ctx.m_brawCtx.getTypeInfo(INT_T).value()), ctx);

        auto movsq = Movsq;
        movs.m_opcode = movsq;
        movs.m_prefix = Rep;
        addInstruction(movs, ctx);
    }

    if(remainder > 0) {
        move(m_registers.at(Register::RCX), std::make_shared<Operands::Immediate>(remainder, ctx.m_brawCtx.getTypeInfo(INT_T).value()), ctx);
        auto movsb = Movsb;
        movs.m_opcode = movsb;
        movs.m_prefix = Rep;
        addInstruction(movs, ctx);
    }
    size_t end = ctx.m_file.m_text.m_instructions.size() - 1;
    std::vector<Instruction> from(ctx.m_file.m_text.m_instructions.begin() + beg, ctx.m_file.m_text.m_instructions.begin() + end + 1);
    std::vector<Instruction> result = MoveResolver::resolve(from, {}, *this, ctx);
    ctx.m_file.m_text.m_instructions.erase(ctx.m_file.m_text.m_instructions.begin() + beg, ctx.m_file.m_text.m_instructions.begin() + end + 1);
    ctx.m_file.m_text.m_instructions.insert(ctx.m_file.m_text.m_instructions.begin() + beg, result.begin(), result.end());


    std::reverse(save.begin(), save.end());
    for(auto reg : save)
        pop(reg, ctx);
}



std::shared_ptr<Operand> CodeGenerator::convertOperand(::Operand source, FunctionContext& ctx) {
    switch(source.index()) {
        case 1:
            return ctx.m_virtualRegisters.at(std::get<std::shared_ptr<::Register>>(source)->m_id);
        case 2: {
            Value v = std::get<Value>(source);

            switch(v.index()) {
                case 0:
                    return std::make_shared<Operands::Immediate>(std::get<int>(v), ctx.m_brawCtx.getTypeInfo(INT_T).value());
                case 1:
                    return std::make_shared<Operands::Immediate>(std::get<long>(v), ctx.m_brawCtx.getTypeInfo(LONG_T).value());
                case 6:
                    return std::make_shared<Operands::Immediate>(std::get<char>(v), ctx.m_brawCtx.getTypeInfo(CHAR_T).value());
                case 2:
                case 3:
                case 5: {
                    static uint32_t id = 0;
                    TypeInfo type = v.index() == 2 ? ctx.m_brawCtx.getTypeInfo(FLOAT_T).value() : v.index() == 3 ? ctx.m_brawCtx.getTypeInfo(DOUBLE_T).value() : ctx.m_brawCtx.getTypeInfo(CHAR_T).value();
                    Label l{"v_" + std::to_string(id)};
                    ctx.m_file.m_data.m_labels.push_back({l.m_id, v});
                    id++;
                    auto la = std::make_shared<Operands::Label>(l.m_id, type);
                    return std::make_shared<Operands::Address>(la, 0, type);
                }
                case 4:
                    return std::make_shared<Operands::Immediate>(std::get<bool>(v), ctx.m_brawCtx.getTypeInfo(BOOL_T).value());
                efault: break;
            }
            break;
        }
        case 3: {
            Address src = std::get<Address>(source);
            auto addrOff = src.m_offset;
            std::shared_ptr<Operands::Address> addr;
            if(ctx.m_virtualRegisters.at(src.m_base->m_id)->m_type == Operand::Type::Address) {
                addr = cast<Operands::Address>(ctx.m_virtualRegisters.at(src.m_base->m_id)->clone());
                if(addr->m_base->m_type == Operand::Type::Register && cast<Operands::Register>(addr->m_base)->m_group == Operands::Register::RBP) 
                    addrOff -= src.m_base->m_type.m_size;
                auto addrType = src.m_base->m_type;
                addr->m_offset = addr->m_offset + addrType.m_size + addrOff;
                auto off = addrOff < 0 ? addrType.m_size + addrOff : addrOff;
                addr->m_typeInfo = src.m_typeInfo;
                addr->m_scale = src.m_scale;
                addr->m_scaleSize = src.m_base->m_scale;
            }
            else {
                addr = std::make_shared<Operands::Address>(ctx.m_virtualRegisters.at(src.m_base->m_id), addrOff, src.m_typeInfo);
                addr->m_scaleSize = src.m_scaleSize;
            }

            if(src.m_index) {
                std::shared_ptr<Operand> index = convertOperand(src.m_index, ctx);
                std::shared_ptr<Operands::Register> indexReg = nullptr;
                if(index->m_type == Operand::Type::Address)
                    indexReg = memoryValueToRegister(cast<Operands::Address>(index), ctx);
                else
                    indexReg = cast<Operands::Register>(index);

                if(addrOff != 0) {
                    std::shared_ptr<Operands::Register> base = m_registers.at(SPILL2);
                    std::shared_ptr<Operands::Address> addr2 = cast<Operands::Address>(addr->clone());
                    addr2->m_typeInfo = Utils::makePointer(addr->m_typeInfo);
                    if(addr2->m_scaleSize > 1)
                        memoryAddressToRegister(addr2, base, ctx);
                    else
                        move(base, addr2, ctx);
                    addr->m_scale = src.m_scale;
                    addr->m_index = indexReg;
                    addr->m_scaleSize = 1;
                    addr = cast<Operands::Address>(addr->clone());
                    addr->m_base = base;
                    addr->m_offset = 0;
                }
                else {
                    addr->m_scale = src.m_scale;
                    addr->m_index = indexReg;
                }
            }
            return addr;
        }
        case 4:
            return std::make_shared<Operands::Label>(std::get<::Label>(source).m_id, Utils::makePointer(TypeInfo{VOID_T, 0, true}));
        default: break;
    }

    return nullptr;
}


bool CodeGenerator::bothAddress(std::shared_ptr<Operand> o1, std::shared_ptr<Operand> o2) const {
    return o1->m_type == Operand::Type::Address && o2->m_type == Operand::Type::Address;
}

bool CodeGenerator::isUnsigned(std::shared_ptr<Operand> o) const {
    return o->m_typeInfo.m_name == UINT_T || o->m_typeInfo.m_name == ULONG_T || o->m_typeInfo.m_name == UCHAR_T;
}

bool CodeGenerator::isFloat(std::shared_ptr<Operand> o) const {
    return o->m_typeInfo.m_name == FLOAT_T;
}

bool CodeGenerator::isDouble(std::shared_ptr<Operand> o) const {
    return o->m_typeInfo.m_name == DOUBLE_T;
}

bool CodeGenerator::isRegisterAlive(Operands::Register::RegisterGroup reg, FunctionContext& ctx) const {
    size_t blockIndex = ctx.m_allocatorResult.m_propagated.blockForInstruction.at(ctx.m_instructionIndex);
    for(auto range : ctx.m_allocatorResult.m_propagated.blocks.at(blockIndex)->m_rangeVector) {
        std::shared_ptr<Operand> arg = ctx.m_virtualRegisters.at(range->m_id);
        if(!(range->m_range.first <= ctx.m_instructionIndex && ctx.m_instructionIndex <= range->m_range.second)/*  || range->m_isAssignedFirst */)
            continue;


        if(arg->m_type != Operand::Type::Register || cast<Operands::Register>(arg)->m_group != reg)
            continue;

        return true;
    }
    return false;
}

void CodeGenerator::initializeRegisters() {
    m_registers.clear();
    m_registers.insert({ Operands::Register::RSP,
        std::shared_ptr<Operands::Register>(new Operands::Register(
            {{Operand::Size::Qword, "rsp"}, {Operand::Size::Dword, "esp"}, {Operand::Size::Word, "sp"}, {Operand::Size::Byte, "spl"}},
            TypeInfo{"void*", 8, true},
            Operands::Register::General, Operands::Register::RSP)
        )
    });
    m_registers.insert({ Operands::Register::RBP,
        std::shared_ptr<Operands::Register>(new Operands::Register(
            {{Operand::Size::Qword, "rbp"}, {Operand::Size::Dword, "ebp"}, {Operand::Size::Word, "bp"}, {Operand::Size::Byte, "bpl"}},
            TypeInfo{"void*", 8, true},
            Operands::Register::General, Operands::Register::RBP)
        )
    });
    m_registers.insert({ Operands::Register::RAX,
        std::shared_ptr<Operands::Register>(new Operands::Register(
            {{Operand::Size::Qword, "rax"}, {Operand::Size::Dword, "eax"}, {Operand::Size::Word, "ax"}, {Operand::Size::Byte, "al"}},
            TypeInfo{},
            Operands::Register::General, Operands::Register::RAX)
        )
    });
    m_registers.insert({ Operands::Register::RDI,
        std::shared_ptr<Operands::Register>(new Operands::Register(
            {{Operand::Size::Qword, "rdi"}, {Operand::Size::Dword, "edi"}, {Operand::Size::Word, "di"}, {Operand::Size::Byte, "dil"}},
            TypeInfo{},
            Operands::Register::General, Operands::Register::RDI)
        )
    });
    m_registers.insert({ Operands::Register::RSI,
        std::shared_ptr<Operands::Register>(new Operands::Register(
            {{Operand::Size::Qword, "rsi"}, {Operand::Size::Dword, "esi"}, {Operand::Size::Word, "si"}, {Operand::Size::Byte, "sil"}},
            TypeInfo{},
            Operands::Register::General, Operands::Register::RSI)
        )
    });
    m_registers.insert({ Operands::Register::RDX,
        std::shared_ptr<Operands::Register>(new Operands::Register(
            {{Operand::Size::Qword, "rdx"}, {Operand::Size::Dword, "edx"}, {Operand::Size::Word, "dx"}, {Operand::Size::Byte, "dl"}},
            TypeInfo{},
            Operands::Register::General, Operands::Register::RDX)
        )
    });
    m_registers.insert({ Operands::Register::RCX,
        std::shared_ptr<Operands::Register>(new Operands::Register(
            {{Operand::Size::Qword, "rcx"}, {Operand::Size::Dword, "ecx"}, {Operand::Size::Word, "cx"}, {Operand::Size::Byte, "cl"}},
            TypeInfo{},
            Operands::Register::General, Operands::Register::RCX)
        )
    });
    m_registers.insert({ Operands::Register::RBX,
        std::shared_ptr<Operands::Register>(new Operands::Register(
            {{Operand::Size::Qword, "rbx"}, {Operand::Size::Dword, "ebx"}, {Operand::Size::Word, "bx"}, {Operand::Size::Byte, "bl"}},
            TypeInfo{},
            Operands::Register::General, Operands::Register::RBX)
        )
    });
    m_registers.insert({ Operands::Register::R8,
        std::shared_ptr<Operands::Register>(new Operands::Register(
            {{Operand::Size::Qword, "r8"}, {Operand::Size::Dword, "r8d"}, {Operand::Size::Word, "r8w"}, {Operand::Size::Byte, "r8b"}},
            TypeInfo{},
            Operands::Register::General, Operands::Register::R8)
        )
    });
    m_registers.insert({ Operands::Register::R9,
        std::shared_ptr<Operands::Register>(new Operands::Register(
            {{Operand::Size::Qword, "r9"}, {Operand::Size::Dword, "r9d"}, {Operand::Size::Word, "r9w"}, {Operand::Size::Byte, "r9b"}},
            TypeInfo{},
            Operands::Register::General, Operands::Register::R9)
        )
    });
    m_registers.insert({ Operands::Register::R10,
        std::shared_ptr<Operands::Register>(new Operands::Register(
            {{Operand::Size::Qword, "r10"}, {Operand::Size::Dword, "r10d"}, {Operand::Size::Word, "r10w"}, {Operand::Size::Byte, "r10b"}},
            TypeInfo{},
            Operands::Register::General, Operands::Register::R10)
        )
    });
    m_registers.insert({ Operands::Register::R11,
        std::shared_ptr<Operands::Register>(new Operands::Register(
            {{Operand::Size::Qword, "r11"}, {Operand::Size::Dword, "r11d"}, {Operand::Size::Word, "r11w"}, {Operand::Size::Byte, "r11b"}},
            TypeInfo{},
            Operands::Register::General, Operands::Register::R11)
        )
    });
    m_registers.insert({ Operands::Register::R12,
        std::shared_ptr<Operands::Register>(new Operands::Register(
            {{Operand::Size::Qword, "r12"}, {Operand::Size::Dword, "r12d"}, {Operand::Size::Word, "r12w"}, {Operand::Size::Byte, "r12b"}},
            TypeInfo{},
            Operands::Register::General, Operands::Register::R12)
        )
    });
    m_registers.insert({ Operands::Register::R13,
        std::shared_ptr<Operands::Register>(new Operands::Register(
            {{Operand::Size::Qword, "r13"}, {Operand::Size::Dword, "r13d"}, {Operand::Size::Word, "r13w"}, {Operand::Size::Byte, "r13b"}},
            TypeInfo{},
            Operands::Register::General, Operands::Register::R13)
        )
    });
    m_registers.insert({ Operands::Register::R14,
        std::shared_ptr<Operands::Register>(new Operands::Register(
            {{Operand::Size::Qword, "r14"}, {Operand::Size::Dword, "r14d"}, {Operand::Size::Word, "r14w"}, {Operand::Size::Byte, "r14b"}},
            TypeInfo{},
            Operands::Register::General, Operands::Register::R14)
        )
    });
    m_registers.insert({ Operands::Register::R15,
        std::shared_ptr<Operands::Register>(new Operands::Register(
            {{Operand::Size::Qword, "r15"}, {Operand::Size::Dword, "r15d"}, {Operand::Size::Word, "r15w"}, {Operand::Size::Byte, "r15b"}},
            TypeInfo{},
            Operands::Register::General, Operands::Register::R15)
        )
    });
    m_registers.insert({ Operands::Register::XMM0,
        std::shared_ptr<Operands::Register>(new Operands::Register(
            {{Operand::Size::Oword, "xmm0"},{Operand::Size::Qword, "xmm0"},{Operand::Size::Dword, "xmm0"}},
            TypeInfo{},
            Operands::Register::Simd, Operands::Register::XMM0)
        )
    });
    m_registers.insert({ Operands::Register::XMM1,
        std::shared_ptr<Operands::Register>(new Operands::Register(
            {{Operand::Size::Oword, "xmm1"},{Operand::Size::Qword, "xmm1"},{Operand::Size::Dword, "xmm1"}},
            TypeInfo{},
            Operands::Register::Simd, Operands::Register::XMM1)
        )
    });
    m_registers.insert({ Operands::Register::XMM2,
        std::shared_ptr<Operands::Register>(new Operands::Register(
            {{Operand::Size::Oword, "xmm2"},{Operand::Size::Qword, "xmm2"},{Operand::Size::Dword, "xmm2"}},
            TypeInfo{},
            Operands::Register::Simd, Operands::Register::XMM2)
        )
    });
    m_registers.insert({ Operands::Register::XMM3,
        std::shared_ptr<Operands::Register>(new Operands::Register(
            {{Operand::Size::Oword, "xmm3"},{Operand::Size::Qword, "xmm3"},{Operand::Size::Dword, "xmm3"}},
            TypeInfo{},
            Operands::Register::Simd, Operands::Register::XMM3)
        )
    });
    m_registers.insert({ Operands::Register::XMM4,
        std::shared_ptr<Operands::Register>(new Operands::Register(
            {{Operand::Size::Oword, "xmm4"},{Operand::Size::Qword, "xmm4"},{Operand::Size::Dword, "xmm4"}},
            TypeInfo{},
            Operands::Register::Simd, Operands::Register::XMM4)
        )
    });
    m_registers.insert({ Operands::Register::XMM5,
        std::shared_ptr<Operands::Register>(new Operands::Register(
            {{Operand::Size::Oword, "xmm5"},{Operand::Size::Qword, "xmm5"},{Operand::Size::Dword, "xmm5"}},
            TypeInfo{},
            Operands::Register::Simd, Operands::Register::XMM5)
        )
    });
    m_registers.insert({ Operands::Register::XMM6,
        std::shared_ptr<Operands::Register>(new Operands::Register(
            {{Operand::Size::Oword, "xmm6"},{Operand::Size::Qword, "xmm6"},{Operand::Size::Dword, "xmm6"}},
            TypeInfo{},
            Operands::Register::Simd, Operands::Register::XMM6)
        )
    });
    m_registers.insert({ Operands::Register::XMM7,
        std::shared_ptr<Operands::Register>(new Operands::Register(
            {{Operand::Size::Oword, "xmm7"},{Operand::Size::Qword, "xmm7"},{Operand::Size::Dword, "xmm7"}},
            TypeInfo{},
            Operands::Register::Simd, Operands::Register::XMM7)
        )
    });
    m_registers.insert({ Operands::Register::XMM8,
        std::shared_ptr<Operands::Register>(new Operands::Register(
            {{Operand::Size::Oword, "xmm8"},{Operand::Size::Qword, "xmm8"},{Operand::Size::Dword, "xmm8"}},
            TypeInfo{},
            Operands::Register::Simd, Operands::Register::XMM8)
        )
    });
    m_registers.insert({ Operands::Register::XMM9,
        std::shared_ptr<Operands::Register>(new Operands::Register(
            {{Operand::Size::Oword, "xmm9"},{Operand::Size::Qword, "xmm9"},{Operand::Size::Dword, "xmm9"}},
            TypeInfo{},
            Operands::Register::Simd, Operands::Register::XMM9)
        )
    });
    m_registers.insert({ Operands::Register::XMM10,
        std::shared_ptr<Operands::Register>(new Operands::Register(
            {{Operand::Size::Oword, "xmm10"},{Operand::Size::Qword, "xmm10"},{Operand::Size::Dword, "xmm10"}},
            TypeInfo{},
            Operands::Register::Simd, Operands::Register::XMM10)
        )
    });
    m_registers.insert({ Operands::Register::XMM11,
        std::shared_ptr<Operands::Register>(new Operands::Register(
            {{Operand::Size::Oword, "xmm11"},{Operand::Size::Qword, "xmm11"},{Operand::Size::Dword, "xmm11"}},
            TypeInfo{},
            Operands::Register::Simd, Operands::Register::XMM11)
        )
    });
    m_registers.insert({ Operands::Register::XMM12,
        std::shared_ptr<Operands::Register>(new Operands::Register(
            {{Operand::Size::Oword, "xmm12"},{Operand::Size::Qword, "xmm12"},{Operand::Size::Dword, "xmm12"}},
            TypeInfo{},
            Operands::Register::Simd, Operands::Register::XMM12)
        )
    });
    m_registers.insert({ Operands::Register::XMM13,
        std::shared_ptr<Operands::Register>(new Operands::Register(
            {{Operand::Size::Oword, "xmm13"},{Operand::Size::Qword, "xmm13"},{Operand::Size::Dword, "xmm13"}},
            TypeInfo{},
            Operands::Register::Simd, Operands::Register::XMM13)
        )
    });
    m_registers.insert({ Operands::Register::XMM14,
        std::shared_ptr<Operands::Register>(new Operands::Register(
            {{Operand::Size::Oword, "xmm14"},{Operand::Size::Qword, "xmm14"},{Operand::Size::Dword, "xmm14"}},
            TypeInfo{},
            Operands::Register::Simd, Operands::Register::XMM14)
        )
    });
    m_registers.insert({ Operands::Register::XMM15,
        std::shared_ptr<Operands::Register>(new Operands::Register(
            {{Operand::Size::Oword, "xmm15"},{Operand::Size::Qword, "xmm15"},{Operand::Size::Dword, "xmm15"}},
            TypeInfo{},
            Operands::Register::Simd, Operands::Register::XMM15)
        )
    });
}

}