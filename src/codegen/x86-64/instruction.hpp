#pragma once

#include "../instruction.hpp"

namespace CodeGen::x86_64 {

enum Opcode : uint16_t {
    Nop = 0x90,
    Ret = 0xC3,
    Add = 0x01,
    Addss = 0xF3 + 0x0F + 0x58,
    Sub = 0x29,
    Subss = 0xF3 + 0x0F + 0x5C,
    Imul = 0x69,
    Mulss = 0xF3 + 0x0F + 0x59,
    Push = 0x50,
    Pop = 0x58,
    Mov = 0x89,
    Movss = 0xF3 + 0x0F + 0x10,
    Jmp = 0xE9
};

struct Instruction : CodeGen::Instruction {
    virtual void opcodeInstruction(std::ostream& os) const override {
        switch((Opcode)m_opcode) {
            case Opcode::Nop: os << "nop"; break;
            case Opcode::Ret: os << "ret"; break;
            case Opcode::Add: os << "add"; break;
            case Opcode::Sub: os << "sub"; break;
            case Opcode::Imul: os << "imul"; break;
            case Opcode::Push: os << "push"; break;
            case Opcode::Pop: os << "pop"; break;
            case Opcode::Mov: os << "mov"; break;
            case Opcode::Jmp: os << "jmp"; break;
            case Opcode::Movss: os << "movss"; break;
            case Opcode::Addss: os << "addss"; break;
            case Opcode::Subss: os << "subss"; break;
            case Opcode::Mulss: os << "mulss"; break;
        }
    }
};

}