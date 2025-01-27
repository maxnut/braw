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
    Movzx = 0x0F + 0xB6,
    Jmp = 0xE9,
    Cmp = 0x3D,
    Sete = 0x0F + 0x94,
    Setne = 0x0F + 0x95,
    Setge = 0x0F + 0x9D,
    Setle = 0x0F + 0x9E,
    Je = 0x0F + 0x84,
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
            case Opcode::Movzx: os << "movzx"; break;
            case Opcode::Cmp: os << "cmp"; break;
            case Opcode::Sete: os << "sete"; break;
            case Opcode::Setne: os << "setne"; break;
            case Opcode::Setge: os << "setge"; break;
            case Opcode::Setle: os << "setle"; break;
            case Opcode::Je: os << "je"; break;
        }
    }
};

}