#include "compiler.hpp"
#include "ir/graph-color/graph_color.hpp"
#include "ir/instruction.hpp"
#include "ir/operator.hpp"
#include "ir/register.hpp"

std::string operatorString(Operator op, const ColorResult& reg) {
    switch(op.index()) {
        case 1: {
            Register r = std::get<Register>(op);
            if(r.m_id == "%return")
                return "rax";
            return reg.m_registers.at(r.m_id); //TODO: spills!
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

void compileMove(const BinaryInstruction* instr, const ColorResult& reg, std::ofstream& fs) {
    fs << "mov " << operatorString(instr->m_left, reg) << ", " << operatorString(instr->m_right, reg) << "\n";
}

void Compiler::compile(const BinaryInstruction* instr, const ColorResult& reg, std::ofstream& fs) {
    switch(instr->m_type) {
        case Instruction::Move:
            return compileMove(instr, reg, fs);
        default:
            break;
    }
}