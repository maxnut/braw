#include "compiler.hpp"
#include "ir/graph-color/graph_color.hpp"
#include "ir/instruction.hpp"
#include "ir/operator.hpp"
#include "ir/register.hpp"

std::string operatorString(Operator op, const CompilerContext& ctx) {
    switch(op.index()) {
        case 1: {
            Register r = std::get<Register>(op);
            if(r.m_id == "%return")
                return "rax";
            return ctx.m_reg.m_registers.at(r.m_id); //TODO: spills!
        }
        case 2: {
            Value value = std::get<Value>(op);

            switch(value.index()) {
                case 0:
                    return std::to_string(std::get<int>(value)); //TODO: access data!
                default:
                    break;
            }
            break;
        }
        default: //TODO: address!
            break;
    }

    return ".err!";
}

void compileMove(const BinaryInstruction* instr, const CompilerContext& ctx, std::ofstream& fs) {
    fs << "mov " << operatorString(instr->m_left, ctx) << ", " << operatorString(instr->m_right, ctx) << "\n";
}

void compileAdd(const BinaryInstruction* instr, const CompilerContext& ctx, std::ofstream& fs) {
    fs << "add " << operatorString(instr->m_left, ctx) << ", " << operatorString(instr->m_right, ctx) << "\n";
}

void compileSubtract(const BinaryInstruction* instr, const CompilerContext& ctx, std::ofstream& fs) {
    fs << "sub " << operatorString(instr->m_left, ctx) << ", " << operatorString(instr->m_right, ctx) << "\n";
}

void compileMultiply(const BinaryInstruction* instr, const CompilerContext& ctx, std::ofstream& fs) {
    fs << "imul " << operatorString(instr->m_left, ctx) << ", " << operatorString(instr->m_right, ctx) << "\n";
}

void Compiler::compile(const BinaryInstruction* instr, const CompilerContext& ctx, std::ofstream& fs) {
    switch(instr->m_type) {
        case Instruction::Move:
            return compileMove(instr, ctx, fs);
        case Instruction::Add:
            return compileAdd(instr, ctx, fs);
        case Instruction::Subtract:
            return compileSubtract(instr, ctx, fs);
        case Instruction::Multiply:
            return compileMultiply(instr, ctx, fs);
        default:
            break;
    }
}