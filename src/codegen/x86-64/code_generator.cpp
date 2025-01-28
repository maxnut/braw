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
#include "ir/value.hpp"
#include <cassert>
#include <cstdint>
#include <memory>
#include <string>

namespace CodeGen::x86_64 {

const char* SPILL1 = "r14";
const char* SPILL2 = "r15";

template <typename T>
std::shared_ptr<T> cast(const std::shared_ptr<void>& ptr) {
    return std::static_pointer_cast<T>(ptr);
}

File CodeGenerator::generate(const ::File& src) {
    File file;

    for(const Function& f : src.m_functions) {
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
                spills += range->size();
            }
        }
        ctx.m_spills = spills;

        for(auto reg : ctx.m_savedRegisters)
            push(reg, ctx);

        if(spills > 0)
            sub(m_registers.at("rsp"), std::make_shared<Operands::Immediate>(spills), ctx);

        for(auto arg : f.m_args) {
            auto op = ctx.m_virtualRegisters.at(arg->m_id);
            if(arg->m_type.m_name == "int")
                op->m_valueType = Operand::ValueType::Signed;
            else if(arg->m_type.m_name == "long")
                op->m_valueType = Operand::ValueType::Signed;
            else if(arg->m_type.m_name == "bool")
                op->m_valueType = Operand::ValueType::Signed;
            else if(arg->m_type.m_name == "char")
                op->m_valueType = Operand::ValueType::Signed;
            else if(arg->m_type.m_name == "float")
                op->m_valueType = Operand::ValueType::SinglePrecision;
            else if(arg->m_type.m_name == "double")
                op->m_valueType = Operand::ValueType::DoublePrecision;
            else
                op->m_valueType = Operand::ValueType::Structure;
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
            compareAndStore(cast<Operands::Register>(convertOperand(bin->m_o1, ctx)), convertOperand(bin->m_o2, ctx), cast<Operands::Register>(convertOperand(bin->m_o3, ctx)), Opcode::Sete, ctx); 
            break;
        }
        case ::Instruction::CompareNotEquals: {
            auto bin = (const ::BasicInstruction*)instr;
            compareAndStore(cast<Operands::Register>(convertOperand(bin->m_o1, ctx)), convertOperand(bin->m_o2, ctx), cast<Operands::Register>(convertOperand(bin->m_o3, ctx)), Opcode::Setne, ctx); 
            break;
        }
        case ::Instruction::CompareGreaterEquals: {
            auto bin = (const ::BasicInstruction*)instr;
            compareAndStore(cast<Operands::Register>(convertOperand(bin->m_o1, ctx)), convertOperand(bin->m_o2, ctx), cast<Operands::Register>(convertOperand(bin->m_o3, ctx)), Opcode::Setge, ctx); 
            break;
        }
        case ::Instruction::CompareLessEquals: {
            auto bin = (const ::BasicInstruction*)instr;
            compareAndStore(cast<Operands::Register>(convertOperand(bin->m_o1, ctx)), convertOperand(bin->m_o2, ctx), cast<Operands::Register>(convertOperand(bin->m_o3, ctx)), Opcode::Setle, ctx); 
            break;
        }
        case ::Instruction::JumpFalse: {
            auto bin = (const ::BasicInstruction*)instr;
            compareAndJump(cast<Operands::Register>(convertOperand(bin->m_o1, ctx)), std::make_shared<Operands::Immediate>(0), cast<Operands::Label>(convertOperand(bin->m_o2, ctx)), Opcode::Je, ctx);
            break;
        }
        case ::Instruction::Call: {
            auto callIn = (const ::CallInstruction*)instr;
            call(std::make_shared<Operands::Label>(callIn->m_id), cast<Operands::Register>(convertOperand(callIn->m_optReturn, ctx)), callIn->m_parameters, ctx);
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
    in.m_opcode = (isRegister(target) && isFloat(target) && source->m_type == Operand::Type::Address) || (isRegister(source) && isFloat(source) && target->m_type == Operand::Type::Address) ? Opcode::Movdqu :
        isFloat(source) ? Opcode::Movss :
        (bothRegisters(target, source) && isSmaller(cast<Operands::Register>(source), cast<Operands::Register>(target))) ? Opcode::Movzx : Opcode::Mov;
    if(!isRegister(target)) target->m_valueType = source->m_valueType; assert(source->m_valueType != Operand::ValueType::Count);
    in.m_operands.push_back(target);
    in.m_operands.push_back(source);
    ctx.f.m_text.m_instructions.push_back(std::move(in));
}

void CodeGenerator::add(std::shared_ptr<Operand> target, std::shared_ptr<Operand> source, FunctionContext& ctx) {
    Instruction in;
    if(bothAddress(target, source)) source = memoryValueToRegister(std::static_pointer_cast<Operands::Address>(source), ctx);
    in.m_opcode = isFloat(source) ? Opcode::Addss : Opcode::Add;
    target->m_valueType = source->m_valueType; assert(source->m_valueType != Operand::ValueType::Count);
    in.m_operands.push_back(target);
    in.m_operands.push_back(source);
    ctx.f.m_text.m_instructions.push_back(std::move(in));
}

void CodeGenerator::sub(std::shared_ptr<Operand> target, std::shared_ptr<Operand> source, FunctionContext& ctx) {
    Instruction in;
    if(bothAddress(target, source)) source = memoryValueToRegister(std::static_pointer_cast<Operands::Address>(source), ctx);
    in.m_opcode = isFloat(source) ? Opcode::Subss : Opcode::Sub;
    target->m_valueType = source->m_valueType; assert(source->m_valueType != Operand::ValueType::Count);
    in.m_operands.push_back(target);
    in.m_operands.push_back(source);
    ctx.f.m_text.m_instructions.push_back(std::move(in));
}

void CodeGenerator::mul(std::shared_ptr<Operand> target, std::shared_ptr<Operand> source, FunctionContext& ctx) {
    Instruction in;
    if(bothAddress(target, source)) source = memoryValueToRegister(std::static_pointer_cast<Operands::Address>(source), ctx);
    in.m_opcode = isFloat(source) ? Opcode::Mulss : Opcode::Imul;
    target->m_valueType = source->m_valueType; assert(source->m_valueType != Operand::ValueType::Count);
    in.m_operands.push_back(target);
    in.m_operands.push_back(source);
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
    i.m_opcode = Opcode::Call;
    i.m_operands.push_back(label);
    ctx.f.m_text.m_instructions.push_back(i);

    if(spilled > 0)
        add(m_registers.at("rsp"), std::make_shared<Operands::Immediate>(spilled * 8), ctx);

    if(optReturn)
        move(optReturn, isFloat(optReturn) ? m_registers.at("xmm0") : m_registers.at("rax"), ctx);

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
    i.m_opcode = Opcode::Ret;
    ctx.f.m_text.m_instructions.push_back(i);
}

void CodeGenerator::push(std::shared_ptr<Operand> target, FunctionContext& ctx) {
    if(target->m_type == Operand::Type::Immediate) {
        Instruction i;
        i.m_opcode = Opcode::Push;
        i.m_operands.push_back(target);
        ctx.f.m_text.m_instructions.push_back(i);
        return;
    }
    else if(isRegister(target)) {
        auto reg = cast<Operands::Register>(target);
        Instruction i;

        if(!isFloat(reg)) {
            i.m_opcode = Opcode::Push;
            i.m_operands.push_back(target);
            ctx.f.m_text.m_instructions.push_back(i);
            return;
        }

        sub(m_registers.at("rsp"), std::make_shared<Operands::Immediate>(16), ctx);
        move(std::make_shared<Operands::Address>(m_registers.at("rsp")), reg, ctx);
        return;
    }

    auto addrReg = memoryAddressToRegister(cast<Operands::Address>(target), ctx);
    Instruction i;
    i.m_opcode = Opcode::Push;
    i.m_operands.push_back(addrReg);
    ctx.f.m_text.m_instructions.push_back(i);
}

void CodeGenerator::pop(std::shared_ptr<Operands::Register> target, FunctionContext& ctx) {
    if(isFloat(target)) {
        move(target, std::make_shared<Operands::Address>(m_registers.at("rsp")), ctx);
        add(m_registers.at("rsp"), std::make_shared<Operands::Immediate>(16), ctx);
        return;
    }
    Instruction i;
    i.m_opcode = Opcode::Pop;
    i.m_operands.push_back(target);
    ctx.f.m_text.m_instructions.push_back(i);
}


std::shared_ptr<Operands::Register> CodeGenerator::memoryValueToRegister(std::shared_ptr<Operands::Address> address, FunctionContext& ctx) {
    move(m_registers[SPILL1], address, ctx);
    return m_registers[SPILL1];
}

std::shared_ptr<Operands::Register> CodeGenerator::memoryAddressToRegister(std::shared_ptr<Operands::Address> address, FunctionContext& ctx) {
    Instruction i;
    i.m_opcode = Opcode::Lea;
    i.m_operands.push_back(m_registers[SPILL1]);
    i.m_operands.push_back(address);
    ctx.f.m_text.m_instructions.push_back(i);
    return m_registers[SPILL1];
}

void CodeGenerator::compareAndStore(std::shared_ptr<Operands::Register> reg, std::shared_ptr<Operand> op, std::shared_ptr<Operands::Register> store, Opcode setOpcode, FunctionContext& ctx) {
    Instruction cmp, set;
    cmp.m_opcode = isFloat(op) || isFloat(reg) ? Opcode::Ucomiss : Opcode::Cmp;
    cmp.m_operands.push_back(reg);
    cmp.m_operands.push_back(op);
    ctx.f.m_text.m_instructions.push_back(cmp);
    set.m_opcode = setOpcode;
    set.m_operands.push_back(m_registers.at("al"));
    ctx.f.m_text.m_instructions.push_back(set);
    move(store, m_registers.at("al"), ctx);
}

void CodeGenerator::compareAndJump(std::shared_ptr<Operands::Register> reg, std::shared_ptr<Operand> op, std::shared_ptr<Operands::Label> label, Opcode jumpOpcode, FunctionContext& ctx) {
    Instruction cmp, jmp;
    cmp.m_opcode = Opcode::Cmp;
    cmp.m_operands.push_back(reg);
    cmp.m_operands.push_back(op);
    ctx.f.m_text.m_instructions.push_back(cmp);
    jmp.m_opcode = jumpOpcode;
    jmp.m_operands.push_back(label);
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
                    Label l{"v_" + std::to_string(id)};
                    ctx.f.m_data.m_labels.push_back({l.m_id, v});
                    id++;
                    auto la = std::make_shared<Operands::Label>(l.m_id);
                    return std::make_shared<Operands::Address>(
                        la, v.index() == 2 ? Operand::ValueType::SinglePrecision : v.index() == 3 ? Operand::ValueType::DoublePrecision : Operand::ValueType::Pointer);
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
            return std::make_shared<Operands::Label>(std::get<::Label>(source).m_id);
        default: break;
    }

    return nullptr;
}


bool CodeGenerator::bothAddress(std::shared_ptr<Operand> o1, std::shared_ptr<Operand> o2) const {
    return o1->m_type == Operand::Type::Address && o2->m_type == Operand::Type::Address;
}

bool CodeGenerator::isFloat(std::shared_ptr<Operand> o) const {
    if(o->m_valueType == Operand::ValueType::SinglePrecision || o->m_valueType == Operand::ValueType::DoublePrecision)
        return true;

    if(o->m_type != Operand::Type::Register)
        return false;

    return cast<Operands::Register>(o)->m_id.contains("xmm");
}

#define REGISTER(ID, size) m_registers[ID] = std::make_shared<Operands::Register>(ID, size)

void CodeGenerator::initializeRegisters() {
    REGISTER("rsp", Operands::Register::Qword);
    REGISTER("rbp", Operands::Register::Qword);
    REGISTER("rax", Operands::Register::Qword);
    REGISTER("rdi", Operands::Register::Qword);
    REGISTER("rsi", Operands::Register::Qword);
    REGISTER("rdx", Operands::Register::Qword);
    REGISTER("rcx", Operands::Register::Qword);
    REGISTER("rbx", Operands::Register::Qword);
    REGISTER("r8", Operands::Register::Qword);
    REGISTER("r9", Operands::Register::Qword);
    REGISTER("r10", Operands::Register::Qword);
    REGISTER("r11", Operands::Register::Qword);
    REGISTER("r12", Operands::Register::Qword);
    REGISTER("r13", Operands::Register::Qword);
    REGISTER("r14", Operands::Register::Qword);
    REGISTER("r15", Operands::Register::Qword);
    REGISTER("xmm0", Operands::Register::Oword);
    REGISTER("xmm1", Operands::Register::Oword);
    REGISTER("xmm2", Operands::Register::Oword);
    REGISTER("xmm3", Operands::Register::Oword);
    REGISTER("xmm4", Operands::Register::Oword);
    REGISTER("xmm5", Operands::Register::Oword);
    REGISTER("xmm6", Operands::Register::Oword);
    REGISTER("xmm7", Operands::Register::Oword);
    REGISTER("xmm8", Operands::Register::Oword);
    REGISTER("xmm9", Operands::Register::Oword);
    REGISTER("xmm10", Operands::Register::Oword);
    REGISTER("xmm11", Operands::Register::Oword);
    REGISTER("xmm12", Operands::Register::Oword);
    REGISTER("xmm13", Operands::Register::Oword);
    REGISTER("xmm14", Operands::Register::Oword);
    REGISTER("xmm15", Operands::Register::Oword);
    REGISTER("al", Operands::Register::Byte);
}

}