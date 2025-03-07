#include "code_generator.hpp"
#include "braw_context.hpp"
#include "codegen/operand.hpp"
#include "codegen/x86-64/address.hpp"
#include "codegen/x86-64/immediate.hpp"
#include "codegen/x86-64/instruction.hpp"
#include "codegen/x86-64/label.hpp"
#include "codegen/x86-64/olabel.hpp"
#include "codegen/x86-64/register.hpp"
#include "cursor.hpp"
#include "ir/address.hpp"
#include "ir/instruction.hpp"
#include "ir/instructions/basic.hpp"
#include "ir/instructions/call.hpp"
#include "ir/label.hpp"
#include "ir/register.hpp"
#include "ir/value.hpp"
#include "rules.hpp"
#include "spdlog/spdlog.h"
#include "type_info.hpp"
#include "utils.hpp"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>

namespace CodeGen::x86_64 {

using Operands::Register;

const Register::RegisterGroup SPILL1 = Register::R15;
const Register::RegisterGroup PRCSPILL1 = Register::XMM15;

template <typename T>
std::shared_ptr<T> cast(const std::shared_ptr<void>& ptr) {
    return std::static_pointer_cast<T>(ptr);
}

void addInstruction(Instruction instr, FunctionContext& ctx) {
    ctx.f.m_text.m_instructions.push_back(std::move(instr));
    ctx.f.m_text.m_instructions.back().m_irIndex = ctx.m_instructionIndex;
    ctx.f.m_text.m_instructions.back().m_irFunctionIndex = ctx.m_functionIndex;
}

File CodeGenerator::generate(const ::File& src, BrawContext& braw) {
    File file;

    size_t idx = 0;
    for(const Function& f : src.m_functions) {
        initializeRegisters();
        file.m_text.m_globals.push_back({f.m_name});

        ColorResult result = GraphColor::build(f, {Register::RDI,Register::RSI,Register::RDX,Register::RCX,Register::R8,Register::R9,Register::RBX,Register::R10,Register::R11,Register::R12,Register::R13,Register::R14,Register::R15}, {Register::XMM0,Register::XMM1,Register::XMM2,Register::XMM3,Register::XMM4,Register::XMM5,Register::XMM6,Register::XMM7,Register::XMM8,Register::XMM9,Register::XMM10,Register::XMM11,Register::XMM12,Register::XMM13,Register::XMM14,Register::XMM15}, 6, 6);
        FunctionContext ctx{file, braw};
        ctx.m_virtualRegisters["%return"] = m_registers.at(Operands::Register::RAX);
        ctx.m_virtualRegisters["%returnF"] = m_registers.at(Operands::Register::XMM0);
        ctx.m_ranges = result.m_rangeVector;
        ctx.m_functionIndex = idx++;

        generate(f.m_instructions.at(0).get(), ctx); //label

        push(m_registers.at(Operands::Register::RBP), ctx);
        move(m_registers.at(Operands::Register::RBP), m_registers.at(Operands::Register::RSP), ctx);

        int64_t spills = 0;
        for(auto range : result.m_rangeVector) {
            if(!range->m_isPointedOrDereferenced && result.m_registers.contains(range->m_id)) {
                ctx.m_virtualRegisters[range->m_id] = range->m_registerType == RegisterType::Struct ? cast<Operand>(std::make_shared<Operands::Address>(m_registers.at(result.m_registers.at(range->m_id)), -range->m_typeInfo.m_size, range->m_typeInfo)) : m_registers.at(result.m_registers.at(range->m_id));
                ctx.m_virtualRegisters[range->m_id]->m_typeInfo = range->m_typeInfo;
                Operands::Register::RegisterGroup group = result.m_registers.at(range->m_id);
                if(group == Operands::Register::RBX || group == Operands::Register::R12 || group == Operands::Register::R13 || group == Operands::Register::R14 || group == Operands::Register::R15)
                    ctx.m_savedRegisters.push_back(m_registers.at(result.m_registers.at(range->m_id)));
            }
            else {
                spills += range->m_typeInfo.m_size;
                ctx.m_virtualRegisters[range->m_id] = std::make_shared<Operands::Address>(m_registers.at(Operands::Register::RBP), -spills, range->m_typeInfo);
                for(auto& arg : f.m_args) {
                    if(arg->m_id == range->m_id) {
                        m_registers.at(result.m_registers.at(range->m_id))->m_typeInfo = range->m_typeInfo;
                        move(ctx.m_virtualRegisters.at(range->m_id), m_registers.at(result.m_registers.at(range->m_id)), ctx);
                        break;
                    }
                }
            }
        }
        spills += spills % 16; // 16 byte alignment
        ctx.m_spills = spills;

        for(auto reg : ctx.m_savedRegisters)
            push(reg, ctx);

        if(spills > 0)
            sub(m_registers.at(Operands::Register::RSP), std::make_shared<Operands::Immediate>(spills, ctx.brawCtx.getTypeInfo("int").value()), ctx);

        //skip label
        for(size_t i = 1; i < f.m_instructions.size(); ++i) {
            auto& in = f.m_instructions[i];
            ctx.m_instructionIndex = i;
            generate(in.get(), ctx);
        }
    }

    return file;
}

void CodeGenerator::generate(const ::Instruction* instr, FunctionContext& ctx) {
    switch(instr->m_type) {
        case ::Instruction::Label:
            ctx.f.m_text.m_labels[ctx.f.m_text.m_instructions.size() + ctx.f.m_text.m_labels.size()] = Label{((const ::Label*)instr)->m_id};
            break;
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
            compareAndJump(cast<Operands::Register>(convertOperand(bin->m_o1, ctx)), std::make_shared<Operands::Immediate>(0, ctx.brawCtx.getTypeInfo("int").value()), cast<Operands::Label>(convertOperand(bin->m_o2, ctx)), Je, ctx);
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
            auto addr1 = cast<Operands::Address>(convertOperand(bin->m_o1, ctx));
            auto addr2 = cast<Operands::Address>(convertOperand(bin->m_o2, ctx));
            copyAddressToAddress(addr1, addr2, std::get<1>(bin->m_o1)->m_type.m_size, ctx);
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
            auto deref = std::make_shared<Operands::Address>(m_registers.at(SPILL1), 0, Utils::getRawType(m_registers.at(SPILL1)->m_typeInfo, ctx.brawCtx).value());
            move(target, deref, ctx);
            break;
        }
        default: break;
    }
}

void CodeGenerator::move(std::shared_ptr<Operand> target, std::shared_ptr<Operand> source, FunctionContext& ctx) {
    Instruction in;
    if(bothAddress(target, source)) source = memoryValueToRegister(cast<Operands::Address>(source), ctx);
    
    if(isFloat(source)) in.m_opcode = Movss;
    else if(isDouble(source)) in.m_opcode = Movsd;
    else if(bothRegisters(target, source) && target->m_typeInfo.m_name != "" && source->m_typeInfo.m_name != "" && target->m_typeInfo.m_size < source->m_typeInfo.m_size)
        in.m_opcode = Movzx;
    else in.m_opcode = Mov;
    
    target->m_typeInfo = source->m_typeInfo;
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
    in.m_opcode = isFloat(source) ? Mulss : isDouble(source) ? Mulsd : Imul;
    in.addOperand(target);
    in.addOperand(source);
    addInstruction(std::move(in), ctx);
}

void CodeGenerator::call(std::shared_ptr<Operands::Label> label, std::shared_ptr<Operands::Register> optReturn, const std::vector<::Operand>& args, size_t skipArgs, FunctionContext& ctx) {
    static const std::unordered_set<std::shared_ptr<Operands::Register>> callerSaved = {m_registers.at(Register::RAX),m_registers.at(Register::RCX),m_registers.at(Register::RDX),m_registers.at(Register::RSI),m_registers.at(Register::RDI),m_registers.at(Register::R8),m_registers.at(Register::R9),m_registers.at(Register::R10),m_registers.at(Register::R11),m_registers.at(Register::XMM0),m_registers.at(Register::XMM1),m_registers.at(Register::XMM2),m_registers.at(Register::XMM3),m_registers.at(Register::XMM4),m_registers.at(Register::XMM5),m_registers.at(Register::XMM6),m_registers.at(Register::XMM7)};
    std::vector<std::shared_ptr<Operands::Register>> saveStack;

    for(auto range : ctx.m_ranges) {
        if(!(range->m_range.first < ctx.m_instructionIndex && ctx.m_instructionIndex < range->m_range.second))
            continue;

        std::shared_ptr<Operand> arg = ctx.m_virtualRegisters.at(range->m_id);

        if(arg->m_type != Operand::Type::Register || !callerSaved.contains(cast<Operands::Register>(arg)))
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

    size_t spilled = 0;
    for(auto& arg : args) {
        auto op = convertOperand(arg, ctx);
        
        if((!isFloat(op) && !cursor.hasNext()) || (isFloat(op) && !floatCursor.hasNext())) {
            push(op, ctx);
            spilled += op->m_typeInfo.m_size;
            ctx.m_spills += op->m_typeInfo.m_size;
            continue;
        }

        if(isFloat(op)) {
            Operands::Register::RegisterGroup reg = floatCursor.get().next().value();
            move(m_registers.at(reg), op, ctx);
        } else {
            auto reg = cursor.get().next().value();
            if(arg.index() == 1 && std::get<1>(arg)->m_registerType == RegisterType::Struct) {
                op = copyAddressToNew(cast<Operands::Address>(op), std::get<1>(arg)->m_type.m_size, ctx);
                spilled += std::get<1>(arg)->m_type.m_size;
            }

            if(op->m_type == Operand::Type::Address) {
                auto addr = cast<Operands::Address>(op->clone());
                addr->m_offset += std::get<1>(arg)->m_type.m_size;
                memoryAddressToRegister(addr, m_registers.at(reg), ctx);
            }
            else
                move(m_registers.at(reg), op, ctx);
        }
    }

    Instruction i;
    i.m_opcode = Call;
    i.addOperand(label);
    addInstruction(i, ctx);

    if(optReturn) {
        auto retReg = isFloat(optReturn) || isDouble(optReturn) ? m_registers.at(Operands::Register::XMM0) : m_registers.at(Operands::Register::RAX);
        move(optReturn, retReg, ctx);
    }

    if(spilled > 0) {
        add(m_registers.at(Operands::Register::RSP), std::make_shared<Operands::Immediate>(spilled, ctx.brawCtx.getTypeInfo("int").value()), ctx);
        ctx.m_spills -= spilled;
    }

    std::reverse(saveStack.begin(), saveStack.end());
    for(auto reg : saveStack)
        pop(reg, ctx);
}

void CodeGenerator::ret(FunctionContext& ctx) {
    if(ctx.m_spills > 0)
        add(m_registers.at(Operands::Register::RSP), std::make_shared<Operands::Immediate>(ctx.m_spills, ctx.brawCtx.getTypeInfo("int").value()), ctx);
    
    std::reverse(ctx.m_savedRegisters.begin(), ctx.m_savedRegisters.end());
    for(auto reg : ctx.m_savedRegisters)
        pop(reg, ctx);
    
    pop(m_registers.at(Operands::Register::RBP), ctx);
    Instruction i;
    i.m_opcode = Ret;
    addInstruction(i, ctx);
}

void CodeGenerator::push(std::shared_ptr<Operand> target, FunctionContext& ctx) {
    if(target->m_type == Operand::Type::Immediate) {
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
            i.m_opcode = Push;
            auto clone = target->clone();
            // TODO find a way to force qword
            i.addOperand(clone);
            addInstruction(i, ctx);
            return;
        }

        sub(m_registers.at(Operands::Register::RSP), std::make_shared<Operands::Immediate>((int)reg->m_typeInfo.m_size, ctx.brawCtx.getTypeInfo("int").value()), ctx);
        move(std::make_shared<Operands::Address>(m_registers.at(Operands::Register::RSP), TypeInfo{}), reg, ctx);
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
        move(target, std::make_shared<Operands::Address>(m_registers.at(Operands::Register::RSP), target->m_typeInfo), ctx);
        add(m_registers.at(Operands::Register::RSP), std::make_shared<Operands::Immediate>((int)target->m_typeInfo.m_size, ctx.brawCtx.getTypeInfo("int").value()), ctx);
        return;
    }
    Instruction i;
    i.m_opcode = Pop;
    auto clone = target->clone();
    // TODO find a way to force qword
    i.addOperand(clone);
    addInstruction(i, ctx);
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

void CodeGenerator::compareAndStore(std::shared_ptr<Operands::Register> reg, std::shared_ptr<Operand> op, std::shared_ptr<Operands::Register> store, InstructionOpcode setOpcode, FunctionContext& ctx) {
    Instruction cmp, set;
    cmp.m_opcode = isFloat(op) ? Ucomiss : isDouble(op) ? Ucomisd : Cmp;
    cmp.addOperand(reg);
    cmp.addOperand(op);
    addInstruction(cmp, ctx);
    set.m_opcode = setOpcode;
    m_registers.at(Operands::Register::RAX)->m_typeInfo = ctx.brawCtx.getTypeInfo("bool").value();
    set.addOperand(m_registers.at(Operands::Register::RAX));
    addInstruction(set, ctx);
    move(store, m_registers.at(Operands::Register::RAX), ctx);
}

void CodeGenerator::compareAndJump(std::shared_ptr<Operands::Register> reg, std::shared_ptr<Operand> op, std::shared_ptr<Operands::Label> label, InstructionOpcode jumpOpcode, FunctionContext& ctx) {
    Instruction cmp, jmp;
    cmp.m_opcode = Cmp;
    cmp.addOperand(reg);
    cmp.addOperand(op);
    addInstruction(cmp, ctx);
    jmp.m_opcode = jumpOpcode;
    jmp.addOperand(label);
    addInstruction(jmp, ctx);
}

std::shared_ptr<Operands::Address> CodeGenerator::copyAddressToNew(std::shared_ptr<Operands::Address> address, size_t size, FunctionContext& ctx) {
    sub(m_registers.at(Operands::Register::RSP), std::make_shared<Operands::Immediate>((int)size, ctx.brawCtx.getTypeInfo("int").value()), ctx);
    auto target = std::make_shared<Operands::Address>(m_registers.at(Operands::Register::RBP), -ctx.m_spills, address->m_typeInfo);
    ctx.m_spills += size;
    copyAddressToAddress(target, address, size, ctx);
    return target;
}

void CodeGenerator::copyAddressToAddressPointer(std::shared_ptr<Operands::Address> target, std::shared_ptr<Operands::Address> source, size_t size, FunctionContext& ctx) {
    std::vector<std::shared_ptr<Operands::Register>> save;

    if(m_registers.at(Register::RDI)->m_typeInfo.m_name != "")
        save.push_back(m_registers.at(Register::RDI));
    if(m_registers.at(Register::RSI)->m_typeInfo.m_name != "")
        save.push_back(m_registers.at(Register::RSI));
    if(m_registers.at(Register::RCX)->m_typeInfo.m_name != "")
        save.push_back(m_registers.at(Register::RCX));

    for(auto reg : save)
        push(reg, ctx);

    size_t remainder = size % 8;

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
        move(m_registers.at(Register::RCX), std::make_shared<Operands::Immediate>((size - remainder) / 8, ctx.brawCtx.getTypeInfo("int").value()), ctx);

        auto movsq = Movsq;
        movsq.prefix = 0xF3;
        movs.m_opcode = movsq;
        addInstruction(movs, ctx);
    }

    if(remainder > 0) {
        move(m_registers.at(Register::RCX), std::make_shared<Operands::Immediate>(remainder, ctx.brawCtx.getTypeInfo("int").value()), ctx);
        auto movsb = Movsb;
        movsb.prefix = 0xF3;
        movs.m_opcode = movsb;
        addInstruction(movs, ctx);
    }

    std::reverse(save.begin(), save.end());
    for(auto reg : save)
        pop(reg, ctx);
}

void CodeGenerator::copyAddressToAddress(std::shared_ptr<Operands::Address> target, std::shared_ptr<Operands::Address> source, size_t size, FunctionContext& ctx) {
    std::vector<std::shared_ptr<Operands::Register>> save;

    if(m_registers.at(Register::RDI)->m_typeInfo.m_name != "")
        save.push_back(m_registers.at(Register::RDI));
    if(m_registers.at(Register::RSI)->m_typeInfo.m_name != "")
        save.push_back(m_registers.at(Register::RSI));
    if(m_registers.at(Register::RCX)->m_typeInfo.m_name != "")
        save.push_back(m_registers.at(Register::RCX));

    for(auto reg : save)
        push(reg, ctx);

    size_t remainder = size % 8;

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
        move(m_registers.at(Register::RCX), std::make_shared<Operands::Immediate>((size - remainder) / 8, ctx.brawCtx.getTypeInfo("int").value()), ctx);

        auto movsq = Movsq;
        movsq.prefix = 0xF3;
        movs.m_opcode = movsq;
        addInstruction(movs, ctx);
    }

    if(remainder > 0) {
        move(m_registers.at(Register::RCX), std::make_shared<Operands::Immediate>(remainder, ctx.brawCtx.getTypeInfo("int").value()), ctx);
        auto movsb = Movsb;
        movsb.prefix = 0xF3;
        movs.m_opcode = movsb;
        addInstruction(movs, ctx);
    }

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
                    return std::make_shared<Operands::Immediate>(std::get<int>(v), ctx.brawCtx.getTypeInfo("int").value());
                case 1:
                    return std::make_shared<Operands::Immediate>(std::get<long>(v), ctx.brawCtx.getTypeInfo("long").value());
                case 2:
                case 3:
                case 5: {
                    static uint32_t id = 0;
                    TypeInfo type = v.index() == 2 ? ctx.brawCtx.getTypeInfo("float").value() : v.index() == 3 ? ctx.brawCtx.getTypeInfo("double").value() : Utils::makePointer(ctx.brawCtx.getTypeInfo("char").value());
                    Label l{"v_" + std::to_string(id)};
                    ctx.f.m_data.m_labels.push_back({l.m_id, v});
                    id++;
                    auto la = std::make_shared<Operands::Label>(l.m_id, type);
                    return std::make_shared<Operands::Address>(la, 0, type);
                }
                case 4:
                    return std::make_shared<Operands::Immediate>(std::get<bool>(v), ctx.brawCtx.getTypeInfo("bool").value());
                efault: break;
            }
            break;
        }
        case 3: {
            auto addrOff = std::get<Address>(source).m_offset;
            auto addr = cast<Operands::Address>(ctx.m_virtualRegisters.at(std::get<Address>(source).m_base->m_id)->clone());
            if(addr->m_base->m_type == Operand::Type::Register && cast<Operands::Register>(addr->m_base)->m_group == Operands::Register::RBP) 
                addrOff -= std::get<Address>(source).m_base->m_type.m_size;
            auto addrType = std::get<Address>(source).m_base->m_type;
            addr->m_offset = addr->m_offset + addrType.m_size + addrOff;
            auto off = addrOff < 0 ? addrType.m_size + addrOff : addrOff;
            addr->m_typeInfo = Rules::isPtr(addrType.m_name) ? Utils::getRawType(addrType, ctx.brawCtx).value() : ctx.brawCtx.getTypeInfo(addrType.memberByOffset(off)->m_type).value();
            return addr;
        }
        case 4:
            return std::make_shared<Operands::Label>(std::get<::Label>(source).m_id, Utils::makePointer(TypeInfo{"void"}));
        default: break;
    }

    return nullptr;
}


bool CodeGenerator::bothAddress(std::shared_ptr<Operand> o1, std::shared_ptr<Operand> o2) const {
    return o1->m_type == Operand::Type::Address && o2->m_type == Operand::Type::Address;
}

bool CodeGenerator::isFloat(std::shared_ptr<Operand> o) const {
    // if(o->m_type == Operand::Type::Address && cast<Operands::Address>(o)->m_base->m_type == Operand::Type::Label)
    //     return isFloat(cast<Operands::Address>(o)->m_base);

    return o->m_typeInfo.m_name == "float";
}

bool CodeGenerator::isDouble(std::shared_ptr<Operand> o) const {
    // if(o->m_type == Operand::Type::Address && cast<Operands::Address>(o)->m_base->m_type == Operand::Type::Label)
    //     return isDouble(cast<Operands::Address>(o)->m_base);

    return o->m_typeInfo.m_name == "double";
}

void CodeGenerator::initializeRegisters() {
    m_registers.clear();
    m_registers.insert({ Operands::Register::RSP,
        std::shared_ptr<Operands::Register>(new Operands::Register(
            {{Operand::Size::Qword, "rsp"}, {Operand::Size::Dword, "esp"}, {Operand::Size::Word, "sp"}, {Operand::Size::Byte, "spl"}},
            TypeInfo{"void*"},
            Operands::Register::General, Operands::Register::RSP)
        )
    });
    m_registers.insert({ Operands::Register::RBP,
        std::shared_ptr<Operands::Register>(new Operands::Register(
            {{Operand::Size::Qword, "rbp"}, {Operand::Size::Dword, "ebp"}, {Operand::Size::Word, "bp"}, {Operand::Size::Byte, "bpl"}},
            TypeInfo{"void*"},
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
            Operands::Register::General, Operands::Register::RSI)
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
            {{Operand::Size::Qword, "rbx"}, {Operand::Size::Dword, "ebx"}, {Operand::Size::Word, "bx"}, {Operand::Size::Byte, "bx"}},
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