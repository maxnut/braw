#include "compiler.hpp"
#include "ir/instruction.hpp"
#include "ir/instructions/binary.hpp"
#include "ir/label.hpp"

void Compiler::compile(const Instruction* instr, const ColorResult& reg, std::ofstream& fs) {
    switch(instr->m_type) {
        case Instruction::Label:
            fs << ((const Label*)instr)->m_id << ":\n";
            break;
        case Instruction::Add:
        case Instruction::Move:
        case Instruction::Subtract:
        case Instruction::Multiply:
        case Instruction::Point:
        case Instruction::CompareEquals:
        case Instruction::CompareGreaterEquals:
        case Instruction::CompareLessEquals:
        case Instruction::CompareNotEquals:
        case Instruction::JumpFalse:
            return compile((const BinaryInstruction*)instr, reg, fs);
        default:
            break;
    }
}