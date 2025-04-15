#include "printer.hpp"
#include "ir/instruction.hpp"
#include "ssa/instruction.hpp"
#include <memory>

namespace SSA {

void Printer::print(std::ostream& out, const File& file) {
    for(auto& func: file.m_functions)
        print(out, func);
}

void Printer::print(std::ostream& out, const Function& function) {
    if(function.m_external)
        out << "ext ";

    out << '(';
    for(int i = 0; i < function.m_args.size(); i++) {
        out << function.m_args[i]->m_id;
        if(i < function.m_args.size() - 1)
            out << ", ";
    }
    out << ")";

    if(function.m_optReturn)
        out << " -> " << function.m_optReturn->m_id;

    out << "\n";

    for(auto& instr : function.m_instructions) {
        print(out, instr.get());
        out << "\n";
    }
}

void Printer::print(std::ostream& out, const Instruction* instr) {
    switch(instr->m_type) {
        default:
            out << "\t";
            break;
        case Instruction::Label:
            break;
    }

    switch (instr->m_type) {
        case Instruction::Phi: {
            const Phi* phi = static_cast<const Phi*>(instr);
            print(out, phi->m_to.get());
            out << " = phi(";
            for(int i = 0; i < phi->m_operands.size(); i++) {
                print(out, phi->m_operands[i].get());
                if(i < phi->m_operands.size() - 1)
                    out << ", ";
            }
            out << ")";
            break;
        }
        case Instruction::Assign: {
            const Assignment* assign = static_cast<const Assignment*>(instr);
            print(out, assign->m_to.get());
            out << " = ";
            print(out, assign->m_operation.get());
            break;
        }
        case Instruction::Allocate: {
            const Allocate* alloc = static_cast<const Allocate*>(instr);
            out << "alloc ";
            print(out, alloc->m_to.get());
            out << " " << alloc->m_size;
            break;
        }
        case Instruction::Call: {
            const Call* call = static_cast<const Call*>(instr);
            if(call->m_optReturn) {
                print(out, call->m_optReturn.get());
                out << " = ";
            }
            out << call->m_id << "(";
            for(int i = 0; i < call->m_parameters.size(); i++) {
                print(out, call->m_parameters[i].get());
                if(i < call->m_parameters.size() - 1)
                    out << ", ";
            }
            out << ")";
            break;
        }
        case Instruction::Return:
            out << "return";
            break;
        case Instruction::JumpFalse: {
            const Jump* jump = static_cast<const Jump*>(instr);
            out << "jumpfalse ";
            print(out, jump->m_check.get());
            out << " " << jump->m_to->m_id;
            break;
        }
        case Instruction::JumpTrue: {
            const Jump* jump = static_cast<const Jump*>(instr);
            out << "jumptrue ";
            print(out, jump->m_check.get());
            out << " " << jump->m_to->m_id;
            break;
        }
        case Instruction::Jump: {
            const Jump* jump = static_cast<const Jump*>(instr);
            out << "jump " << jump->m_to->m_id;
            break;
        }
        case Instruction::Label: {
            const Label* label = static_cast<const Label*>(instr);
            out << label->m_id << ":";
            break;
        }
        case Instruction::WriteMem: {
            const WriteMem* write = static_cast<const WriteMem*>(instr);
            out << "wmem ";
            print(out, write->m_to.get());
            out << ", ";
            print(out, write->m_value.get());
            break;
        }
        }
    }
    void Printer::print(std::ostream& out, const Operation* oper) {
        switch(oper->m_type) {
        case Operation::Add: out << "add "; break;
        case Operation::Subtract: out << "sub "; break;
        case Operation::Multiply: out << "mul "; break;
        case Operation::Divide: out << "div "; break;
        case Operation::Point: out << "point "; break;
        case Operation::Dereference: out << "deref "; break;
        case Operation::PartialDereference: out << "pderef "; break;
        case Operation::Upsize: out << "upsize "; break;
        case Operation::Downsize: out << "downsize "; break;
        case Operation::CompareEquals: out << "ceq "; break;
        case Operation::CompareNotEquals: out << "cne "; break;
        case Operation::CompareGreaterEquals: out << "cge "; break;
        case Operation::CompareLessEquals: out << "cle "; break;
        case Operation::CompareGreater: out << "cgt "; break;
        case Operation::CompareLess: out << "clt "; break;
        case Operation::Modulo: out << "mod "; break;
        case Operation::And: out << "and "; break;
        case Operation::Or: out << "or "; break;
        case Operation::Xor: out << "xor "; break;
        case Operation::LogicalNot: out << "not "; break;
        case Operation::Load: out << "load "; break;
        case Operation::Reference: out << "ref "; break;
        break;
        }
        if(oper->m_o1)
            print(out, oper->m_o1.get());
        if(oper->m_o2) {
            out << ", ";
            print(out, oper->m_o2.get());
        }
    }
    void Printer::print(std::ostream& out, const Operand* op) {
        switch(op->m_type) {
        case Operand::Register: {
            const Register* reg = static_cast<const Register*>(op);
            out << reg->m_id;
            break;
        }
        case Operand::Immediate: {
            const Immediate* imm = static_cast<const Immediate*>(op);
            switch(imm->m_value.index()) {
            case 0:
                out << std::to_string(std::get<int>(imm->m_value)); break;
            case 1:
                out << std::to_string(std::get<long>(imm->m_value)); break;
            case 2:
                out << std::to_string(std::get<float>(imm->m_value)); break;
            case 3:
                out << std::to_string(std::get<double>(imm->m_value)); break;
            case 4:
                out << std::to_string(std::get<bool>(imm->m_value)); break;
            case 5:
                out << std::get<std::string>(imm->m_value); break;
            case 6:
                out << "NULL"; break;
            default:
                out << ""; break;
            }
            break;
        }
        case Operand::Address: {
            const Address* add = static_cast<const Address*>(op);
            if(add->m_index)
                out << "[" + add->m_base->m_id + "+" + std::to_string(add->m_scale) + "*" + add->m_index->m_id + "+" + std::to_string(add->m_offset) + "]";
            else
                out << "[" + add->m_base->m_id + "+" + std::to_string(add->m_offset) + "]";
            break;
        }
    }
}

}