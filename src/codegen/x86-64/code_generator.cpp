#include "code_generator.hpp"
#include "codegen/operand.hpp"
#include "codegen/x86-64/address.hpp"
#include "codegen/x86-64/immediate.hpp"
#include "codegen/x86-64/instruction.hpp"
#include "codegen/x86-64/label.hpp"
#include "codegen/x86-64/olabel.hpp"
#include "codegen/x86-64/register.hpp"
#include "cursor.hpp"
#include "ir/graph-color/graph_color.hpp"
#include "ir/instruction.hpp"
#include "ir/instructions/basic.hpp"
#include "ir/instructions/call.hpp"
#include "ir/label.hpp"
#include "ir/register.hpp"
#include "ir/value.hpp"
#include <cassert>
#include <cstdint>
#include <memory>
#include <string>

namespace CodeGen::x86_64 {

const char* SPILL1 = "r15";
const char* PRCSPILL1 = "xmm15";

template <typename T>
std::shared_ptr<T> cast(const std::shared_ptr<void>& ptr) {
    return std::static_pointer_cast<T>(ptr);
}

File CodeGenerator::generate(const ::File& src) {
    File file;

    for(const Function& f : src.m_functions) {
        initializeRegisters();
        file.m_text.m_globals.push_back({f.m_name});
        ColorResult result = GraphColor::build(f, {"rdi","rsi","rdx","rcx","r8","r9","rbx","r10","r11","r12","r13","r14","r15"}, {"xmm0","xmm1","xmm2","xmm3","xmm4","xmm5","xmm5","xmm6","xmm7","xmm8","xmm9","xmm10","xmm11","xmm12","xmm13","xmm14","xmm15"});
        FunctionContext ctx{file};
        ctx.m_virtualRegisters["%return"] = m_registers.at("rax");
        ctx.m_virtualRegisters["%returnF"] = m_registers.at("xmm0");
        ctx.m_ranges = result.m_rangeVector;

        generate(f.m_instructions.at(0).get(), ctx); //label

        push(m_registers.at("rbp"), ctx);
        move(m_registers.at("rbp"), m_registers.at("rsp"), ctx);

        size_t spills = 0;
        for(Range* range : result.m_rangeVector) {
            if(result.m_registers.contains(range->m_id)) {
                ctx.m_virtualRegisters[range->m_id] = m_registers.at(result.m_registers.at(range->m_id));

                if(range->m_id == "rbx" || range->m_id == "r12" || range->m_id == "r13" || range->m_id == "r14" || range->m_id == "r15")
                    ctx.m_savedRegisters.push_back(m_registers.at(range->m_id));
            }
            else {
                ctx.m_virtualRegisters[range->m_id] = std::make_shared<Operands::Address>(m_registers.at("rsp"), spills);
                spills += range->m_size;
            }
        }
        ctx.m_spills = spills;

        for(auto reg : ctx.m_savedRegisters)
            push(reg, ctx);

        if(spills > 0)
            sub(m_registers.at("rsp"), std::make_shared<Operands::Immediate>(spills), ctx);

        for(auto arg : f.m_args) {
            auto op = ctx.m_virtualRegisters.at(arg->m_id);
            if(arg->m_type.m_name == "int") {
                op->setValueType(Operand::ValueType::Signed);
                op->setSize(Operand::Size::Dword);
            }
            else if(arg->m_type.m_name == "long") {
                op->setValueType(Operand::ValueType::Signed);
                op->setSize(Operand::Size::Qword);
            }
            else if(arg->m_type.m_name == "bool") {
                op->setValueType(Operand::ValueType::Signed);
                op->setSize(Operand::Size::Byte);
            }
            else if(arg->m_type.m_name == "char") {
                op->setValueType(Operand::ValueType::Signed);
                op->setSize(Operand::Size::Byte);
            }
            else if(arg->m_type.m_name == "float") {
                op->setValueType(Operand::ValueType::SinglePrecision);
                op->setSize(Operand::Size::Dword);
            }
            else if(arg->m_type.m_name == "double") {
                op->setValueType(Operand::ValueType::DoublePrecision);
                op->setSize(Operand::Size::Qword);
            }
            else {
                op->setValueType(Operand::ValueType::Pointer);
                op->setSize(Operand::Size::Qword);
            }
        }

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
            compareAndJump(cast<Operands::Register>(convertOperand(bin->m_o1, ctx)), std::make_shared<Operands::Immediate>(0), cast<Operands::Label>(convertOperand(bin->m_o2, ctx)), Je, ctx);
            break;
        }
        case ::Instruction::Call: {
            auto callIn = (const ::CallInstruction*)instr;
            auto optRet = cast<Operands::Register>(convertOperand(callIn->m_optReturn, ctx));
            
            if(callIn->m_optReturn->m_registerType == RegisterType::Single) optRet->setValueType(Operand::ValueType::SinglePrecision);
            else if(callIn->m_optReturn->m_registerType == RegisterType::Double) optRet->setValueType(Operand::ValueType::DoublePrecision);
            else optRet->setValueType(Operand::ValueType::Signed);

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
    else if(bothRegisters(target, source) && cast<Operands::Register>(target)->getSize() > cast<Operands::Register>(source)->getSize()) in.m_opcode = Movzx;
    else in.m_opcode = Mov;
    
    moveValueType(target, source, ctx);
    target->setSize(source->getSize());
    in.m_operands.push_back(target->clone());
    in.m_operands.push_back(source->clone());
    ctx.f.m_text.m_instructions.push_back(std::move(in));
}

void CodeGenerator::add(std::shared_ptr<Operand> target, std::shared_ptr<Operand> source, FunctionContext& ctx) {
    Instruction in;
    if(bothAddress(target, source)) source = memoryValueToRegister(std::static_pointer_cast<Operands::Address>(source), ctx);
    in.m_opcode = isFloat(source) ? Addss : isDouble(source) ? Addsd : Add;
    in.m_operands.push_back(target->clone());
    in.m_operands.push_back(source->clone());
    ctx.f.m_text.m_instructions.push_back(std::move(in));
}

void CodeGenerator::sub(std::shared_ptr<Operand> target, std::shared_ptr<Operand> source, FunctionContext& ctx) {
    Instruction in;
    if(bothAddress(target, source)) source = memoryValueToRegister(std::static_pointer_cast<Operands::Address>(source), ctx);
    in.m_opcode = isFloat(source) ? Subss : isDouble(source) ? Subsd : Sub;
    in.m_operands.push_back(target->clone());
    in.m_operands.push_back(source->clone());
    ctx.f.m_text.m_instructions.push_back(std::move(in));
}

void CodeGenerator::mul(std::shared_ptr<Operand> target, std::shared_ptr<Operand> source, FunctionContext& ctx) {
    Instruction in;
    if(bothAddress(target, source)) source = memoryValueToRegister(std::static_pointer_cast<Operands::Address>(source), ctx);
    in.m_opcode = isFloat(source) ? Mulss : isDouble(source) ? Mulsd : Imul;
    in.m_operands.push_back(target->clone());
    in.m_operands.push_back(source->clone());
    ctx.f.m_text.m_instructions.push_back(std::move(in));
}

void CodeGenerator::call(std::shared_ptr<Operands::Label> label, std::shared_ptr<Operands::Register> optReturn, const std::vector<::Operand>& args, FunctionContext& ctx) {
    static const std::unordered_set<std::string> callerSaved = {"rax","rcx","rdx","rsi","rdi","r8","r9","r10","r11","xmm0","xmm1","xmm2","xmm3","xmm4","xmm5","xmm6","xmm7"};
    std::vector<std::shared_ptr<Operands::Register>> saveStack;

    for(Range* range : ctx.m_ranges) {
        if(!(range->m_range.first < ctx.m_instructionIndex && ctx.m_instructionIndex < range->m_range.second))
            continue;

        std::shared_ptr<Operand> arg = ctx.m_virtualRegisters.at(range->m_id);

        if(arg->m_type != Operand::Type::Register || !callerSaved.contains(cast<Operands::Register>(arg)->m_id))
            continue;

        auto reg = cast<Operands::Register>(arg);

        saveStack.push_back(reg);
        push(reg, ctx);
    }

    std::array<std::string, 6> parameterRegisters = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
    Cursor<std::array<std::string, 6>::iterator> cursor(parameterRegisters.begin(), parameterRegisters.end());
    std::array<std::string, 6> floatParameterRegisters = {"xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5"};
    Cursor<std::array<std::string, 6>::iterator> floatCursor(floatParameterRegisters.begin(), floatParameterRegisters.end());

    size_t spilled = 0;
    for(auto& arg : args) {
        auto op = convertOperand(arg, ctx);
        
        if(!cursor.hasNext() && !floatCursor.hasNext()) {
            push(op, ctx);
            spilled++;
            continue;
        }

        if(isFloat(op)) {
            auto reg = floatCursor.get().next().value();
            move(m_registers.at(reg), op, ctx);
            push(m_registers.at(reg), ctx);
        } else {
            auto reg = cursor.get().next().value();
            move(m_registers.at(reg), op, ctx);
            push(m_registers.at(reg), ctx);
        }
    }

    Instruction i;
    i.m_opcode = Call;
    i.m_operands.push_back(label->clone());
    ctx.f.m_text.m_instructions.push_back(i);

    if(spilled > 0)
        add(m_registers.at("rsp"), std::make_shared<Operands::Immediate>(spilled * 8), ctx);

    if(optReturn)
        move(optReturn, isFloat(optReturn) || isDouble(optReturn) ? m_registers.at("xmm0") : m_registers.at("rax"), ctx);

    for(auto reg : saveStack)
        pop(reg, ctx);
}

void CodeGenerator::ret(FunctionContext& ctx) {
    if(ctx.m_spills > 0)
        add(m_registers.at("rsp"), std::make_shared<Operands::Immediate>(ctx.m_spills), ctx);
    
    for(auto reg : ctx.m_savedRegisters)
        pop(reg, ctx);
    
    pop(m_registers.at("rbp"), ctx);
    Instruction i;
    i.m_opcode = Ret;
    ctx.f.m_text.m_instructions.push_back(i);
}

void CodeGenerator::push(std::shared_ptr<Operand> target, FunctionContext& ctx) {
    if(target->m_type == Operand::Type::Immediate) {
        Instruction i;
        i.m_opcode = Push;
        i.m_operands.push_back(target->clone());
        ctx.f.m_text.m_instructions.push_back(i);
        return;
    }
    else if(isRegister(target)) {
        auto reg = cast<Operands::Register>(target);
        Instruction i;

        if(!isFloat(reg) && !isDouble(reg)) {
            i.m_opcode = Push;
            i.m_operands.push_back(target->clone());
            ctx.f.m_text.m_instructions.push_back(i);
            return;
        }

        sub(m_registers.at("rsp"), std::make_shared<Operands::Immediate>((int)reg->getSize()), ctx);
        move(std::make_shared<Operands::Address>(m_registers.at("rsp")), reg, ctx);
        return;
    }

    auto addrReg = memoryAddressToRegister(cast<Operands::Address>(target), ctx);
    Instruction i;
    i.m_opcode = Push;
    i.m_operands.push_back(addrReg->clone());
    ctx.f.m_text.m_instructions.push_back(i);
}

void CodeGenerator::pop(std::shared_ptr<Operands::Register> target, FunctionContext& ctx) {
    if(isFloat(target) || isDouble(target)) {
        move(target, std::make_shared<Operands::Address>(m_registers.at("rsp")), ctx);
        add(m_registers.at("rsp"), std::make_shared<Operands::Immediate>((int)target->getSize()), ctx);
        return;
    }
    Instruction i;
    i.m_opcode = Pop;
    i.m_operands.push_back(target->clone());
    ctx.f.m_text.m_instructions.push_back(i);
}


std::shared_ptr<Operands::Register> CodeGenerator::memoryValueToRegister(std::shared_ptr<Operands::Address> address, FunctionContext& ctx) {
    auto reg = isFloat(address) || isDouble(address) ? m_registers.at(SPILL1) : m_registers.at(PRCSPILL1);
    move(reg, address, ctx);
    return reg;
}

std::shared_ptr<Operands::Register> CodeGenerator::memoryAddressToRegister(std::shared_ptr<Operands::Address> address, FunctionContext& ctx) {
    auto reg = isFloat(address) || isDouble(address) ? m_registers.at(SPILL1) : m_registers.at(PRCSPILL1);
    Instruction i;
    i.m_opcode = Lea;
    i.m_operands.push_back(reg->clone());
    i.m_operands.push_back(address->clone());
    ctx.f.m_text.m_instructions.push_back(i);
    reg->setValueType(address->getValueType());
    return reg;
}

void CodeGenerator::compareAndStore(std::shared_ptr<Operands::Register> reg, std::shared_ptr<Operand> op, std::shared_ptr<Operands::Register> store, InstructionOpcode setOpcode, FunctionContext& ctx) {
    Instruction cmp, set;
    cmp.m_opcode = isFloat(op) || isFloat(reg) ? Ucomiss : Cmp;
    cmp.m_operands.push_back(reg->clone());
    cmp.m_operands.push_back(op->clone());
    ctx.f.m_text.m_instructions.push_back(cmp);
    set.m_opcode = setOpcode;
    m_registers.at("al")->setValueType(Operand::ValueType::Signed);
    set.m_operands.push_back(m_registers.at("al")->clone());
    ctx.f.m_text.m_instructions.push_back(set);
    move(store, m_registers.at("al"), ctx);
}

void CodeGenerator::compareAndJump(std::shared_ptr<Operands::Register> reg, std::shared_ptr<Operand> op, std::shared_ptr<Operands::Label> label, InstructionOpcode jumpOpcode, FunctionContext& ctx) {
    Instruction cmp, jmp;
    cmp.m_opcode = Cmp;
    cmp.m_operands.push_back(reg->clone());
    cmp.m_operands.push_back(op->clone());
    ctx.f.m_text.m_instructions.push_back(cmp);
    jmp.m_opcode = jumpOpcode;
    jmp.m_operands.push_back(label->clone());
    ctx.f.m_text.m_instructions.push_back(jmp);
}


std::shared_ptr<Operand> CodeGenerator::convertOperand(::Operand source, FunctionContext& ctx) {
    switch(source.index()) {
        case 1:
            return ctx.m_virtualRegisters.at(std::get<std::shared_ptr<Register>>(source)->m_id);
        case 2: {
            Value v = std::get<Value>(source);

            switch(v.index()) {
                case 0:
                    return std::make_shared<Operands::Immediate>(std::get<int>(v));
                case 1:
                    return std::make_shared<Operands::Immediate>(std::get<long>(v));
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
                    return std::make_shared<Operands::Immediate>(std::get<bool>(v));
                default: break;
            }
            break;
        }
        case 3: {
            auto reg = ctx.m_virtualRegisters[std::get<Address>(source).m_base->m_id];
            return std::make_shared<Operands::Address>(reg, std::get<Address>(source).m_offset);
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

#define REGISTERT(ID, size, vtype, type) m_registers[ID] = std::make_shared<Operands::Register>(ID, size, vtype, type)
#define REGISTER(ID, size, type) m_registers[ID] = std::make_shared<Operands::Register>(ID, size, Operand::ValueType::Count, type)

void CodeGenerator::initializeRegisters() {
    m_registers.clear();
    REGISTERT("rsp", Operands::Register::Qword, Operand::ValueType::Pointer, Operands::Register::General);
    REGISTER("rbp", Operands::Register::Qword, Operands::Register::General);
    REGISTER("rax", Operands::Register::Qword, Operands::Register::General);
    REGISTER("rdi", Operands::Register::Qword, Operands::Register::General);
    REGISTER("rsi", Operands::Register::Qword, Operands::Register::General);
    REGISTER("rdx", Operands::Register::Qword, Operands::Register::General);
    REGISTER("rcx", Operands::Register::Qword, Operands::Register::General);
    REGISTER("rbx", Operands::Register::Qword, Operands::Register::General);
    REGISTER("r8", Operands::Register::Qword, Operands::Register::General);
    REGISTER("r9", Operands::Register::Qword, Operands::Register::General);
    REGISTER("r10", Operands::Register::Qword, Operands::Register::General);
    REGISTER("r11", Operands::Register::Qword, Operands::Register::General);
    REGISTER("r12", Operands::Register::Qword, Operands::Register::General);
    REGISTER("r13", Operands::Register::Qword, Operands::Register::General);
    REGISTER("r14", Operands::Register::Qword, Operands::Register::General);
    REGISTER("r15", Operands::Register::Qword, Operands::Register::General);
    REGISTER("xmm0", Operands::Register::Oword, Operands::Register::Simd);
    REGISTER("xmm1", Operands::Register::Oword, Operands::Register::Simd);
    REGISTER("xmm2", Operands::Register::Oword, Operands::Register::Simd);
    REGISTER("xmm3", Operands::Register::Oword, Operands::Register::Simd);
    REGISTER("xmm4", Operands::Register::Oword, Operands::Register::Simd);
    REGISTER("xmm5", Operands::Register::Oword, Operands::Register::Simd);
    REGISTER("xmm6", Operands::Register::Oword, Operands::Register::Simd);
    REGISTER("xmm7", Operands::Register::Oword, Operands::Register::Simd);
    REGISTER("xmm8", Operands::Register::Oword, Operands::Register::Simd);
    REGISTER("xmm9", Operands::Register::Oword, Operands::Register::Simd);
    REGISTER("xmm10", Operands::Register::Oword, Operands::Register::Simd);
    REGISTER("xmm11", Operands::Register::Oword, Operands::Register::Simd);
    REGISTER("xmm12", Operands::Register::Oword, Operands::Register::Simd);
    REGISTER("xmm13", Operands::Register::Oword, Operands::Register::Simd);
    REGISTER("xmm14", Operands::Register::Oword, Operands::Register::Simd);
    REGISTER("xmm15", Operands::Register::Oword, Operands::Register::Simd);
    REGISTER("al", Operands::Register::Byte, Operands::Register::General);
}

}