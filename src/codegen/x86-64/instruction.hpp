#pragma once

#include "../instruction.hpp"
#include "codegen/operand.hpp"
#include <map>
#include <unordered_map>

namespace CodeGen::x86_64 {

enum Opcode {
    Nop,
    Ret,
    Add,
    Addss,
    Addsd,
    Sub,
    Subss,
    Subsd,
    Imul,
    Mul,
    Mulss,
    Mulsd,
    Push,
    Pop,
    Mov,
    Movss,
    Movsd,
    Movzx,
    Movsx,
    Jmp,
    Cmp,
    Ucomiss,
    Ucomisd,
    Sete,
    Setne,
    Setge,
    Setle,
    Setl,
    Setg,
    Je,
    Jne,
    Call,
    Movdqu,
    Lea,
    Movsq,
    Movsb,
    Cdqe,
    Movsxd,
    Cvtss2sd,
    Idiv,
    Div,
    Divss,
    Divsd,
    Rcpps,
    Cqo,
    Cdq,
    And,
    Or,
    Xor,
    Test,
    LabelOp,
    Shr,
    Sar,
    Shl
};

enum Prefix {
    None,
    Rep
};

inline const char* opcodeToString(Opcode opcode) {
    switch(opcode) {
    case Nop: return "nop";
    case Ret: return "ret";
    case Add: return "add";
    case Addss: return "addss";
    case Addsd: return "addsd";
    case Sub: return "sub";
    case Subss: return "subss";
    case Subsd: return "subsd";
    case Imul: return "imul";
    case Mul: return "mul";
    case Mulss: return "mulss";
    case Mulsd: return "mulsd";
    case Push: return "push";
    case Pop: return "pop";
    case Mov: return "mov";
    case Movss: return "movss";
    case Movsd: return "movsd";
    case Movzx: return "movzx";
    case Movsx: return "movsx";
    case Jmp: return "jmp";
    case Cmp: return "cmp";
    case Ucomiss: return "ucomiss";
    case Ucomisd: return "ucomisd";
    case Sete: return "sete";
    case Setne: return "setne";
    case Setge: return "setge";
    case Setle: return "setle";
    case Setg: return "setg";
    case Setl: return "setl";
    case Je: return "je";
    case Jne: return "jne";
    case Call: return "call";
    case Movdqu: return "movdqu";
    case Lea: return "lea";
    case Movsq: return "movsq";
    case Movsb: return "movsb";
    case Cdqe: return "cdqe";
    case Movsxd: return "movsxd";
    case Cvtss2sd: return "cvtss2sd";
    case Idiv: return "idiv";
    case Div: return "div";
    case Divss: return "divss";
    case Divsd: return "divsd";
    case Rcpps: return "rcpps";
    case Cqo: return "cqo";
    case Cdq: return "cdq";
    case And: return "and";
    case Or: return "or";
    case Xor: return "xor";
    case Test: return "test";
    case Shr: return "shr";
    case Sar: return "shr";
    case Shl: return "shl";
    case LabelOp:
      break;
    }
    return "";
}

inline const char* prefixToString(Prefix prefix) {
    switch(prefix) {
    case Rep: return "rep ";
    case None: break;
    }
    return "";
}

struct Instruction : CodeGen::Instruction {

    Opcode m_opcode;
    Prefix m_prefix = None;

};

}