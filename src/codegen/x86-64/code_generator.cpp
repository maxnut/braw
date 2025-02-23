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
#include "ir/instruction.hpp"
#include "ir/instructions/basic.hpp"
#include "ir/instructions/call.hpp"
#include "ir/label.hpp"
#include "ir/register.hpp"
#include "ir/value.hpp"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <memory>
#include <string>

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

void setOperandInfo(std::shared_ptr<Operand> op, const TypeInfo& info) {
    if(info.m_name == "int") {
        op->setValueType(Operand::ValueType::Signed);
        op->setSize(Operand::Size::Dword);
    }
    else if(info.m_name == "long") {
        op->setValueType(Operand::ValueType::Signed);
        op->setSize(Operand::Size::Qword);
    }
    else if(info.m_name == "bool") {
        op->setValueType(Operand::ValueType::Signed);
        op->setSize(Operand::Size::Byte);
    }
    else if(info.m_name == "char") {
        op->setValueType(Operand::ValueType::Signed);
        op->setSize(Operand::Size::Byte);
    }
    else if(info.m_name == "float") {
        op->setValueType(Operand::ValueType::SinglePrecision);
        op->setSize(Operand::Size::Dword);
    }
    else if(info.m_name == "double") {
        op->setValueType(Operand::ValueType::DoublePrecision);
        op->setSize(Operand::Size::Qword);
    }
    else {
        op->setValueType(Operand::ValueType::Pointer);
        op->setSize(Operand::Size::Qword);
    }
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

        size_t spills = 0;
        for(auto range : result.m_rangeVector) {
            if(result.m_registers.contains(range->m_id)) {
                ctx.m_virtualRegisters[range->m_id] = range->m_registerType == RegisterType::Struct ? cast<Operand>(std::make_shared<Operands::Address>(m_registers.at(result.m_registers.at(range->m_id)), 0)) : m_registers.at(result.m_registers.at(range->m_id));

                if(range->m_id == "rbx" || range->m_id == "r12" || range->m_id == "r13" || range->m_id == "r14" || range->m_id == "r15")
                    ctx.m_savedRegisters.push_back(m_registers.at(result.m_registers.at(range->m_id)));
            }
            else {
                ctx.m_virtualRegisters[range->m_id] = std::make_shared<Operands::Address>(m_registers.at(Operands::Register::RBP), -spills - range->m_typeInfo.m_size);
                spills += range->m_typeInfo.m_size;
            }

            setOperandInfo(ctx.m_virtualRegisters[range->m_id], range->m_typeInfo);
        }
        spills += spills % 16; // 16 byte alignment
        ctx.m_spills = spills;

        for(auto reg : ctx.m_savedRegisters)
            push(reg, ctx);

        if(spills > 0)
            sub(m_registers.at(Operands::Register::RSP), std::make_shared<Operands::Immediate>(spills, Operand::ValueType::Signed, Operand::Size::Qword), ctx);

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
            ctx.f.m_text.m_labels[ctx.f.m_text.m_instructions.size()] = Label{((const ::Label*)instr)->m_id};
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
            compareAndJump(cast<Operands::Register>(convertOperand(bin->m_o1, ctx)), std::make_shared<Operands::Immediate>(0, Operand::ValueType::Signed, Operand::Size::Qword), cast<Operands::Label>(convertOperand(bin->m_o2, ctx)), Je, ctx);
            break;
        }
        case ::Instruction::Call: {
            auto callIn = (const ::CallInstruction*)instr;
            auto optRet = callIn->m_optReturn ? cast<Operands::Register>(convertOperand(callIn->m_optReturn, ctx)) : nullptr;
            setOperandInfo(optRet, callIn->m_returnType);
            // if(optRet && callIn->m_optReturn->m_registerType == RegisterType::Single) optRet->setValueType(Operand::ValueType::SinglePrecision);
            // else if(optRet && callIn->m_optReturn->m_registerType == RegisterType::Double) optRet->setValueType(Operand::ValueType::DoublePrecision);
            // else if(optRet) optRet->setValueType(Operand::ValueType::Signed);

            call(std::make_shared<Operands::Label>(callIn->m_id, Operand::ValueType::Pointer, Operand::Size::Qword), optRet, callIn->m_parameters, ctx);
            break;
        }
        case ::Instruction::Return:
            return ret(ctx);
        default: break;
    }
}

void CodeGenerator::move(std::shared_ptr<Operand> target, std::shared_ptr<Operand> source, FunctionContext& ctx) {
    Instruction in;
    if(bothAddress(target, source)) source = memoryValueToRegister(cast<Operands::Address>(source), ctx);
    
    if(isFloat(source)) in.m_opcode = Movss;
    else if(isDouble(source)) in.m_opcode = Movsd;
    else if(bothRegisters(target, source) && cast<Operands::Register>(target)->getSize() > cast<Operands::Register>(source)->getSize())
        in.m_opcode = Movzx;
    else in.m_opcode = Mov;
    
    moveValueType(target, source, ctx);
    target->setSize(source->getSize());
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

void CodeGenerator::call(std::shared_ptr<Operands::Label> label, std::shared_ptr<Operands::Register> optReturn, const std::vector<::Operand>& args, FunctionContext& ctx) {
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

    size_t spilled = 0;
    for(auto& arg : args) {
        auto op = convertOperand(arg, ctx);
        
        if((!isFloat(op) && !cursor.hasNext()) || (isFloat(op) && !floatCursor.hasNext())) {
            push(op, ctx);
            spilled += op->getSize();
            ctx.m_spills += op->getSize();
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

            if(op->m_type == Operand::Type::Address)
                memoryAddressToRegister(cast<Operands::Address>(op), m_registers.at(reg), ctx);
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
        moveValueType(retReg, optReturn, ctx);
        retReg->setSize(optReturn->getSize());
        move(optReturn, retReg, ctx);
    }

    if(spilled > 0) {
        add(m_registers.at(Operands::Register::RSP), std::make_shared<Operands::Immediate>(spilled, Operand::ValueType::Signed, Operand::Size::Qword), ctx);
        ctx.m_spills -= spilled;
    }

    std::reverse(saveStack.begin(), saveStack.end());
    for(auto reg : saveStack)
        pop(reg, ctx);
}

void CodeGenerator::ret(FunctionContext& ctx) {
    if(ctx.m_spills > 0)
        add(m_registers.at(Operands::Register::RSP), std::make_shared<Operands::Immediate>(ctx.m_spills,  Operand::ValueType::Signed, Operand::Size::Qword), ctx);
    
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
            clone->setSize(Operand::Size::Qword);
            i.addOperand(clone);
            addInstruction(i, ctx);
            return;
        }

        sub(m_registers.at(Operands::Register::RSP), std::make_shared<Operands::Immediate>((int)reg->getSize(), Operand::ValueType::Signed, Operand::Size::Qword), ctx);
        move(std::make_shared<Operands::Address>(m_registers.at(Operands::Register::RSP)), reg, ctx);
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
        move(target, std::make_shared<Operands::Address>(m_registers.at(Operands::Register::RSP)), ctx);
        add(m_registers.at(Operands::Register::RSP), std::make_shared<Operands::Immediate>((int)target->getSize(), Operand::ValueType::Signed, Operand::Size::Qword), ctx);
        return;
    }
    Instruction i;
    i.m_opcode = Pop;
    auto clone = target->clone();
    clone->setSize(Operand::Size::Qword);
    i.addOperand(clone);
    addInstruction(i, ctx);
}


std::shared_ptr<Operands::Register> CodeGenerator::memoryValueToRegister(std::shared_ptr<Operands::Address> address, FunctionContext& ctx) {
    auto reg = isFloat(address) || isDouble(address) ? m_registers.at(PRCSPILL1) : m_registers.at(SPILL1);
    move(reg, address, ctx);
    return reg;
}

std::shared_ptr<Operands::Register> CodeGenerator::memoryAddressToRegister(std::shared_ptr<Operands::Address> address, FunctionContext& ctx) {
    auto reg = isFloat(address) || isDouble(address) ? m_registers.at(PRCSPILL1) : m_registers.at(SPILL1);
    return memoryAddressToRegister(address, reg, ctx);
}


std::shared_ptr<Operands::Register> CodeGenerator::memoryAddressToRegister(std::shared_ptr<Operands::Address> address, std::shared_ptr<Operands::Register> reg, FunctionContext& ctx) {
    Instruction i;
    i.m_opcode = Lea;
    auto clone = cast<Operands::Register>(reg->clone());
    clone->setValueType(address->getValueType());
    clone->setSize(address->getSize());
    i.addOperand(clone);
    i.addOperand(address);
    addInstruction(i, ctx);
    return clone;
}

void CodeGenerator::compareAndStore(std::shared_ptr<Operands::Register> reg, std::shared_ptr<Operand> op, std::shared_ptr<Operands::Register> store, InstructionOpcode setOpcode, FunctionContext& ctx) {
    Instruction cmp, set;
    cmp.m_opcode = isFloat(op) ? Ucomiss : isDouble(op) ? Ucomisd : Cmp;
    cmp.addOperand(reg);
    cmp.addOperand(op);
    addInstruction(cmp, ctx);
    set.m_opcode = setOpcode;
    m_registers.at(Operands::Register::RAX)->setValueType(Operand::ValueType::Signed);
    m_registers.at(Operands::Register::RAX)->setSize(Operand::Size::Byte);
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
    sub(m_registers.at(Operands::Register::RSP), std::make_shared<Operands::Immediate>((int)size, Operand::ValueType::Signed, Operand::Size::Qword), ctx);
    auto target = std::make_shared<Operands::Address>(m_registers.at(Operands::Register::RBP), -ctx.m_spills);
    ctx.m_spills += size;
    copyAddressToAddress(target, address, size, ctx);
    return target;
}

void CodeGenerator::copyAddressToAddress(std::shared_ptr<Operands::Address> target, std::shared_ptr<Operands::Address> source, size_t size, FunctionContext& ctx) {
    std::vector<std::shared_ptr<Operands::Register>> save;

    if(m_registers.at(Register::RDI)->getSize() != Operand::Size::Uninitialized)
        save.push_back(m_registers.at(Register::RDI));
    if(m_registers.at(Register::RSI)->getSize() != Operand::Size::Uninitialized)
        save.push_back(m_registers.at(Register::RSI));
    if(m_registers.at(Register::RCX)->getSize() != Operand::Size::Uninitialized)
        save.push_back(m_registers.at(Register::RCX));

    for(auto reg : save)
        push(reg, ctx);

    size_t remainder = size % 8;

    Instruction lea;
    lea.m_opcode = Lea;
    Operand::Size p = m_registers.at(Register::RDI)->getSize();
    m_registers.at(Register::RDI)->setSize(Operand::Size::Qword);
    lea.addOperand(m_registers.at(Register::RDI));
    lea.addOperand(target);
    addInstruction(lea, ctx);
    m_registers.at(Register::RDI)->setSize(p);
    lea.m_operands.clear();
    p = m_registers.at(Register::RSI)->getSize();
    m_registers.at(Register::RSI)->setSize(Operand::Size::Qword);
    lea.addOperand(m_registers.at(Register::RSI));
    lea.m_operands.at(0)->setSize(Operand::Size::Qword);
    lea.addOperand(source);
    addInstruction(lea, ctx);
    m_registers.at(Register::RSI)->setSize(p);
    move(m_registers.at(Register::RCX), std::make_shared<Operands::Immediate>((size - remainder) / 8, Operand::ValueType::Signed, Operand::Size::Qword), ctx);

    Instruction movs;
    auto movsq = Movsq;
    movsq.prefix = 0xF3;
    movs.m_opcode = movsq;
    addInstruction(movs, ctx);

    if(remainder > 0) {
        move(m_registers.at(Register::RCX), std::make_shared<Operands::Immediate>(remainder, Operand::ValueType::Signed, Operand::Size::Qword), ctx);
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
                    return std::make_shared<Operands::Immediate>(std::get<int>(v), Operand::ValueType::Signed, Operand::Size::Dword);
                case 1:
                    return std::make_shared<Operands::Immediate>(std::get<long>(v), Operand::ValueType::Signed, Operand::Size::Qword);
                case 2:
                case 3:
                case 5: {
                    static uint32_t id = 0;
                    Operand::ValueType vt = v.index() == 2 ? Operand::ValueType::SinglePrecision : v.index() == 3 ? Operand::ValueType::DoublePrecision : Operand::ValueType::Pointer;
                    Operand::Size sz = v.index() == 2 ? Operand::Size::Dword : v.index() == 3 ? Operand::Size::Qword : Operand::Size::Qword;
                    Label l{"v_" + std::to_string(id)};
                    ctx.f.m_data.m_labels.push_back({l.m_id, v});
                    id++;
                    auto la = std::make_shared<Operands::Label>(l.m_id, vt, sz);
                    la->setValueType(v.index() == 2 ? Operand::ValueType::SinglePrecision : v.index() == 3 ? Operand::ValueType::DoublePrecision : Operand::ValueType::Pointer);
                    return std::make_shared<Operands::Address>(la);
                }
                case 4:
                    return std::make_shared<Operands::Immediate>(std::get<bool>(v), Operand::ValueType::Signed, Operand::Size::Byte);
                default: break;
            }
            break;
        }
        case 3: {
            auto addr = cast<Operands::Address>(ctx.m_virtualRegisters.at(std::get<Address>(source).m_base->m_id));
            addr->m_offset = std::get<Address>(source).m_offset;
            setOperandInfo(addr, ctx.brawCtx.getTypeInfo(std::get<Address>(source).m_base->m_type.memberByOffset(-addr->m_offset)->m_type).value());
            return addr;
        }
        case 4:
            return std::make_shared<Operands::Label>(std::get<::Label>(source).m_id, Operand::ValueType::Pointer, Operand::Size::Qword);
        default: break;
    }

    return nullptr;
}


bool CodeGenerator::bothAddress(std::shared_ptr<Operand> o1, std::shared_ptr<Operand> o2) const {
    return o1->m_type == Operand::Type::Address && o2->m_type == Operand::Type::Address;
}

bool CodeGenerator::isFloat(std::shared_ptr<Operand> o) const {
    return o->getValueType() == Operand::ValueType::SinglePrecision;
}

bool CodeGenerator::isDouble(std::shared_ptr<Operand> o) const {
    return o->getValueType() == Operand::ValueType::DoublePrecision;
}

void CodeGenerator::moveValueType(std::shared_ptr<Operand> target, std::shared_ptr<Operand> source, FunctionContext& ctx) {
    assert(source->getValueType() != Operand::ValueType::Count);
    target->setValueType(source->getValueType());
}

void CodeGenerator::initializeRegisters() {
    m_registers.clear();
    m_registers.insert({ Operands::Register::RSP,
        std::shared_ptr<Operands::Register>(new Operands::Register(
            {{Operand::Size::Qword, "rsp"}, {Operand::Size::Dword, "esp"}, {Operand::Size::Word, "sp"}, {Operand::Size::Byte, "spl"}},
            Operand::Size::Qword,
            Operand::ValueType::Pointer,
            Operands::Register::General)
        )
    });
    m_registers.insert({ Operands::Register::RBP,
        std::shared_ptr<Operands::Register>(new Operands::Register(
            {{Operand::Size::Qword, "rbp"}, {Operand::Size::Dword, "ebp"}, {Operand::Size::Word, "bp"}, {Operand::Size::Byte, "bpl"}},
            Operand::Size::Qword,
            Operand::ValueType::Pointer,
            Operands::Register::General)
        )
    });
    m_registers.insert({ Operands::Register::RAX,
        std::shared_ptr<Operands::Register>(new Operands::Register(
            {{Operand::Size::Qword, "rax"}, {Operand::Size::Dword, "eax"}, {Operand::Size::Word, "ax"}, {Operand::Size::Byte, "al"}},
            Operand::Size::Uninitialized,
            Operand::ValueType::Count,
            Operands::Register::General)
        )
    });
    m_registers.insert({ Operands::Register::RDI,
        std::shared_ptr<Operands::Register>(new Operands::Register(
            {{Operand::Size::Qword, "rdi"}, {Operand::Size::Dword, "edi"}, {Operand::Size::Word, "di"}, {Operand::Size::Byte, "dil"}},
            Operand::Size::Uninitialized,
            Operand::ValueType::Count,
            Operands::Register::General)
        )
    });
    m_registers.insert({ Operands::Register::RSI,
        std::shared_ptr<Operands::Register>(new Operands::Register(
            {{Operand::Size::Qword, "rsi"}, {Operand::Size::Dword, "esi"}, {Operand::Size::Word, "si"}, {Operand::Size::Byte, "sil"}},
            Operand::Size::Uninitialized,
            Operand::ValueType::Count,
            Operands::Register::General)
        )
    });
    m_registers.insert({ Operands::Register::RDX,
        std::shared_ptr<Operands::Register>(new Operands::Register(
            {{Operand::Size::Qword, "rdx"}, {Operand::Size::Dword, "edx"}, {Operand::Size::Word, "dx"}, {Operand::Size::Byte, "dl"}},
            Operand::Size::Uninitialized,
            Operand::ValueType::Count,
            Operands::Register::General)
        )
    });
    m_registers.insert({ Operands::Register::RCX,
        std::shared_ptr<Operands::Register>(new Operands::Register(
            {{Operand::Size::Qword, "rcx"}, {Operand::Size::Dword, "ecx"}, {Operand::Size::Word, "cx"}, {Operand::Size::Byte, "cl"}},
            Operand::Size::Uninitialized,
            Operand::ValueType::Count,
            Operands::Register::General)
        )
    });
    m_registers.insert({ Operands::Register::RBX,
        std::shared_ptr<Operands::Register>(new Operands::Register(
            {{Operand::Size::Qword, "rbx"}, {Operand::Size::Dword, "ebx"}, {Operand::Size::Word, "bx"}, {Operand::Size::Byte, "bx"}},
            Operand::Size::Uninitialized,
            Operand::ValueType::Count,
            Operands::Register::General)
        )
    });
    m_registers.insert({ Operands::Register::R8,
        std::shared_ptr<Operands::Register>(new Operands::Register(
            {{Operand::Size::Qword, "r8"}, {Operand::Size::Dword, "r8d"}, {Operand::Size::Word, "r8w"}, {Operand::Size::Byte, "r8b"}},
            Operand::Size::Uninitialized,
            Operand::ValueType::Count,
            Operands::Register::General)
        )
    });
    m_registers.insert({ Operands::Register::R9,
        std::shared_ptr<Operands::Register>(new Operands::Register(
            {{Operand::Size::Qword, "r9"}, {Operand::Size::Dword, "r9d"}, {Operand::Size::Word, "r9w"}, {Operand::Size::Byte, "r9b"}},
            Operand::Size::Uninitialized,
            Operand::ValueType::Count,
            Operands::Register::General)
        )
    });
    m_registers.insert({ Operands::Register::R10,
        std::shared_ptr<Operands::Register>(new Operands::Register(
            {{Operand::Size::Qword, "r10"}, {Operand::Size::Dword, "r10d"}, {Operand::Size::Word, "r10w"}, {Operand::Size::Byte, "r10b"}},
            Operand::Size::Uninitialized,
            Operand::ValueType::Count,
            Operands::Register::General)
        )
    });
    m_registers.insert({ Operands::Register::R11,
        std::shared_ptr<Operands::Register>(new Operands::Register(
            {{Operand::Size::Qword, "r11"}, {Operand::Size::Dword, "r11d"}, {Operand::Size::Word, "r11w"}, {Operand::Size::Byte, "r11b"}},
            Operand::Size::Uninitialized,
            Operand::ValueType::Count,
            Operands::Register::General)
        )
    });
    m_registers.insert({ Operands::Register::R12,
        std::shared_ptr<Operands::Register>(new Operands::Register(
            {{Operand::Size::Qword, "r12"}, {Operand::Size::Dword, "r12d"}, {Operand::Size::Word, "r12w"}, {Operand::Size::Byte, "r12b"}},
            Operand::Size::Uninitialized,
            Operand::ValueType::Count,
            Operands::Register::General)
        )
    });
    m_registers.insert({ Operands::Register::R13,
        std::shared_ptr<Operands::Register>(new Operands::Register(
            {{Operand::Size::Qword, "r13"}, {Operand::Size::Dword, "r13d"}, {Operand::Size::Word, "r13w"}, {Operand::Size::Byte, "r13b"}},
            Operand::Size::Uninitialized,
            Operand::ValueType::Count,
            Operands::Register::General)
        )
    });
    m_registers.insert({ Operands::Register::R14,
        std::shared_ptr<Operands::Register>(new Operands::Register(
            {{Operand::Size::Qword, "r14"}, {Operand::Size::Dword, "r14d"}, {Operand::Size::Word, "r14w"}, {Operand::Size::Byte, "r14b"}},
            Operand::Size::Uninitialized,
            Operand::ValueType::Count,
            Operands::Register::General)
        )
    });
    m_registers.insert({ Operands::Register::R15,
        std::shared_ptr<Operands::Register>(new Operands::Register(
            {{Operand::Size::Qword, "r15"}, {Operand::Size::Dword, "r15d"}, {Operand::Size::Word, "r15w"}, {Operand::Size::Byte, "r15b"}},
            Operand::Size::Uninitialized,
            Operand::ValueType::Count,
            Operands::Register::General)
        )
    });
    m_registers.insert({ Operands::Register::XMM0,
        std::shared_ptr<Operands::Register>(new Operands::Register(
            {{Operand::Size::Oword, "xmm0"},{Operand::Size::Qword, "xmm0"},{Operand::Size::Dword, "xmm0"}},
            Operand::Size::Uninitialized,
            Operand::ValueType::Count,
            Operands::Register::Simd)
        )
    });
    m_registers.insert({ Operands::Register::XMM1,
        std::shared_ptr<Operands::Register>(new Operands::Register(
            {{Operand::Size::Oword, "xmm1"},{Operand::Size::Qword, "xmm1"},{Operand::Size::Dword, "xmm1"}},
            Operand::Size::Uninitialized,
            Operand::ValueType::Count,
            Operands::Register::Simd)
        )
    });
    m_registers.insert({ Operands::Register::XMM2,
        std::shared_ptr<Operands::Register>(new Operands::Register(
            {{Operand::Size::Oword, "xmm2"},{Operand::Size::Qword, "xmm2"},{Operand::Size::Dword, "xmm2"}},
            Operand::Size::Uninitialized,
            Operand::ValueType::Count,
            Operands::Register::Simd)
        )
    });
    m_registers.insert({ Operands::Register::XMM3,
        std::shared_ptr<Operands::Register>(new Operands::Register(
            {{Operand::Size::Oword, "xmm3"},{Operand::Size::Qword, "xmm3"},{Operand::Size::Dword, "xmm3"}},
            Operand::Size::Uninitialized,
            Operand::ValueType::Count,
            Operands::Register::Simd)
        )
    });
    m_registers.insert({ Operands::Register::XMM4,
        std::shared_ptr<Operands::Register>(new Operands::Register(
            {{Operand::Size::Oword, "xmm4"},{Operand::Size::Qword, "xmm4"},{Operand::Size::Dword, "xmm4"}},
            Operand::Size::Uninitialized,
            Operand::ValueType::Count,
            Operands::Register::Simd)
        )
    });
    m_registers.insert({ Operands::Register::XMM5,
        std::shared_ptr<Operands::Register>(new Operands::Register(
            {{Operand::Size::Oword, "xmm5"},{Operand::Size::Qword, "xmm5"},{Operand::Size::Dword, "xmm5"}},
            Operand::Size::Uninitialized,
            Operand::ValueType::Count,
            Operands::Register::Simd)
        )
    });
    m_registers.insert({ Operands::Register::XMM6,
        std::shared_ptr<Operands::Register>(new Operands::Register(
            {{Operand::Size::Oword, "xmm6"},{Operand::Size::Qword, "xmm6"},{Operand::Size::Dword, "xmm6"}},
            Operand::Size::Uninitialized,
            Operand::ValueType::Count,
            Operands::Register::Simd)
        )
    });
    m_registers.insert({ Operands::Register::XMM7,
        std::shared_ptr<Operands::Register>(new Operands::Register(
            {{Operand::Size::Oword, "xmm7"},{Operand::Size::Qword, "xmm7"},{Operand::Size::Dword, "xmm7"}},
            Operand::Size::Uninitialized,
            Operand::ValueType::Count,
            Operands::Register::Simd)
        )
    });
    m_registers.insert({ Operands::Register::XMM8,
        std::shared_ptr<Operands::Register>(new Operands::Register(
            {{Operand::Size::Oword, "xmm8"},{Operand::Size::Qword, "xmm8"},{Operand::Size::Dword, "xmm8"}},
            Operand::Size::Uninitialized,
            Operand::ValueType::Count,
            Operands::Register::Simd)
        )
    });
    m_registers.insert({ Operands::Register::XMM9,
        std::shared_ptr<Operands::Register>(new Operands::Register(
            {{Operand::Size::Oword, "xmm9"},{Operand::Size::Qword, "xmm9"},{Operand::Size::Dword, "xmm9"}},
            Operand::Size::Uninitialized,
            Operand::ValueType::Count,
            Operands::Register::Simd)
        )
    });
    m_registers.insert({ Operands::Register::XMM10,
        std::shared_ptr<Operands::Register>(new Operands::Register(
            {{Operand::Size::Oword, "xmm10"},{Operand::Size::Qword, "xmm10"},{Operand::Size::Dword, "xmm10"}},
            Operand::Size::Uninitialized,
            Operand::ValueType::Count,
            Operands::Register::Simd)
        )
    });
    m_registers.insert({ Operands::Register::XMM11,
        std::shared_ptr<Operands::Register>(new Operands::Register(
            {{Operand::Size::Oword, "xmm11"},{Operand::Size::Qword, "xmm11"},{Operand::Size::Dword, "xmm11"}},
            Operand::Size::Uninitialized,
            Operand::ValueType::Count,
            Operands::Register::Simd)
        )
    });
    m_registers.insert({ Operands::Register::XMM12,
        std::shared_ptr<Operands::Register>(new Operands::Register(
            {{Operand::Size::Oword, "xmm12"},{Operand::Size::Qword, "xmm12"},{Operand::Size::Dword, "xmm12"}},
            Operand::Size::Uninitialized,
            Operand::ValueType::Count,
            Operands::Register::Simd)
        )
    });
    m_registers.insert({ Operands::Register::XMM13,
        std::shared_ptr<Operands::Register>(new Operands::Register(
            {{Operand::Size::Oword, "xmm13"},{Operand::Size::Qword, "xmm13"},{Operand::Size::Dword, "xmm13"}},
            Operand::Size::Uninitialized,
            Operand::ValueType::Count,
            Operands::Register::Simd)
        )
    });
    m_registers.insert({ Operands::Register::XMM14,
        std::shared_ptr<Operands::Register>(new Operands::Register(
            {{Operand::Size::Oword, "xmm14"},{Operand::Size::Qword, "xmm14"},{Operand::Size::Dword, "xmm14"}},
            Operand::Size::Uninitialized,
            Operand::ValueType::Count,
            Operands::Register::Simd)
        )
    });
    m_registers.insert({ Operands::Register::XMM15,
        std::shared_ptr<Operands::Register>(new Operands::Register(
            {{Operand::Size::Oword, "xmm15"},{Operand::Size::Qword, "xmm15"},{Operand::Size::Dword, "xmm15"}},
            Operand::Size::Uninitialized,
            Operand::ValueType::Count,
            Operands::Register::Simd)
        )
    });
}

}