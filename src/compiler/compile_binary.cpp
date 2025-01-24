#include "compiler.hpp"
#include "ir/graph-color/graph_color.hpp"
#include "ir/instruction.hpp"
#include "ir/label.hpp"
#include "ir/operator.hpp"
#include "ir/register.hpp"
#include <memory>

std::string operatorString(Operator op, const CompilerContext& ctx) {
    switch(op.index()) {
        case 1: {
            auto r = std::get<std::shared_ptr<Register>>(op);
            if(r->m_id == "%return")
                return "rax";
            return ctx.m_reg.m_registers.at(r->m_id); //TODO: spills!
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
        case 4:
            return std::get<Label>(op).m_id;
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

void compileCompareEquals(const BinaryInstruction* instr, const CompilerContext& ctx, std::ofstream& fs) {
    fs << "cmp " << operatorString(instr->m_left, ctx) << ", " << operatorString(instr->m_right, ctx) << "\n";
    fs << "sete al\n";
    fs << "mov " << operatorString(instr->m_left, ctx) << ", al\n";
}

void compileCompareNotEquals(const BinaryInstruction* instr, const CompilerContext& ctx, std::ofstream& fs) {
    fs << "cmp " << operatorString(instr->m_left, ctx) << ", " << operatorString(instr->m_right, ctx) << "\n";
    fs << "setne al\n";
    fs << "mov " << operatorString(instr->m_left, ctx) << ", al\n";
}

void compileCompareGreaterEquals(const BinaryInstruction* instr, const CompilerContext& ctx, std::ofstream& fs) {
    fs << "cmp " << operatorString(instr->m_left, ctx) << ", " << operatorString(instr->m_right, ctx) << "\n";
    fs << "setge al\n";
    fs << "mov " << operatorString(instr->m_left, ctx) << ", al\n";
}

void compileCompareLessEquals(const BinaryInstruction* instr, const CompilerContext& ctx, std::ofstream& fs) {
    fs << "cmp " << operatorString(instr->m_left, ctx) << ", " << operatorString(instr->m_right, ctx) << "\n";
    fs << "setle al\n";
    fs << "movzx " << operatorString(instr->m_left, ctx) << ", al\n";
}

void compileJumpFalse(const BinaryInstruction* instr, const CompilerContext& ctx, std::ofstream& fs) {
    fs << "cmp " << operatorString(instr->m_left, ctx) << ", 0\n";
    fs << "je " << operatorString(instr->m_right, ctx) << "\n";
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
        case Instruction::CompareEquals:
            return compileCompareEquals(instr, ctx, fs);
        case Instruction::CompareNotEquals:
            return compileCompareNotEquals(instr, ctx, fs);
        case Instruction::CompareGreaterEquals:
            return compileCompareGreaterEquals(instr, ctx, fs);
        case Instruction::CompareLessEquals:
            return compileCompareLessEquals(instr, ctx, fs);
        case Instruction::JumpFalse:
            return compileJumpFalse(instr, ctx, fs);
        default:
            break;
    }
}