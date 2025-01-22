#include "ir_printer.hpp"
#include "ir/instruction.hpp"
#include <string>
#include <variant>

void IRPrinter::print(std::ostream& out, const File& file) {
    for(auto& func: file.m_functions)
        print(out, func);
}

void IRPrinter::print(std::ostream& out, const Function& function) {
    out << '(';
    for(int i = 0; i < function.m_args.size(); i++) {
        out << function.m_args[i].m_id;
        if(i < function.m_args.size() - 1)
            out << ", ";
    }
    out << ")";

    if(function.m_optReturn.m_id != "")
        out << " -> " << function.m_optReturn.m_id;

    out << "\n";

    for(auto& instr : function.m_instructions) {
        switch(instr->m_type) {
            case Instruction::Label:
                out << static_cast<const Label*>(instr.get())->m_id << ":\n";
                break;
            case Instruction::Return:
                out << "    return\n";
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
                print(out, static_cast<const BinaryInstruction*>(instr.get()));
                break;
            default:
                break;
        }
    }
}

std::string operatorString(Operator op) {
    switch(op.index()) {
        case 1:
            return std::get<Register>(op).m_id;
        case 2: {
            Value value = std::get<Value>(op);

            switch(value.index()) {
                case 0:
                    return std::to_string(std::get<int>(value));
                case 1:
                    return std::to_string(std::get<long>(value));
                case 2:
                    return std::to_string(std::get<float>(value));
                case 3:
                    return std::to_string(std::get<double>(value));
                case 4:
                    return std::to_string(std::get<bool>(value));
                case 5:
                    return std::get<std::string>(value);
                case 6:
                    return "NULL";
                default:
                    return "";
            }

        }
        case 3: {
            Address add = std::get<Address>(op);
            return "[" + add.m_base.m_id + "+" + std::to_string(add.m_offset) + "]";
        }
        case 4:
            return std::get<std::string>(op);
        case 0:
        default:
            break;
    }

    return "";
}

void IRPrinter::print(std::ostream& out, const BinaryInstruction* instr) {
    out << "    ";
    switch(instr->m_type) {
        case Instruction::Add:
            out << "add ";
            break;
        case Instruction::Subtract:
            out << "sub ";
            break;
        case Instruction::Multiply:
            out << "mult ";
            break;
        case Instruction::Move:
            out << "move ";
            break;
        case Instruction::Point:
            out << "point ";
            break;
        case Instruction::CompareEquals:
            out << "cmpe ";
            break;
        case Instruction::CompareNotEquals:
            out << "cmpne ";
            break;
        case Instruction::CompareGreaterEquals:
            out << "cmpge ";
            break;
        case Instruction::CompareLessEquals:
            out << "cmple ";
            break;
        case Instruction::JumpFalse:
            out << "jmpf ";
            break;
        default:
            return;
    }

    out << operatorString(instr->m_left) << ", " << operatorString(instr->m_right) << "\n";
}