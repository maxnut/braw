#pragma once

#include "../instruction.hpp"
#include "codegen/operand.hpp"
#include <map>
#include <unordered_map>

namespace CodeGen::x86_64 {

struct InstructionOpcode {
    uint8_t prefix;
    uint8_t opcode1;
    uint8_t opcode2;
    bool usesPrefix = false;

    constexpr InstructionOpcode(uint8_t p, uint8_t o1, uint8_t o2 = 0, bool pfx = false)
        : prefix(p), opcode1(o1), opcode2(o2), usesPrefix(pfx) {}

    bool operator<(const InstructionOpcode& other) const {
        return std::tie(prefix, opcode1, opcode2) < std::tie(other.prefix, other.opcode1, other.opcode2);
    }

    bool operator!=(const InstructionOpcode& other) const {
        return std::tie(prefix, opcode1, opcode2) != std::tie(other.prefix, other.opcode1, other.opcode2);
    }
    bool operator==(const InstructionOpcode& other) const {
        return std::tie(prefix, opcode1, opcode2) == std::tie(other.prefix, other.opcode1, other.opcode2);
    }
};

constexpr InstructionOpcode Nop      = {0x00, 0x90};          // NOP
constexpr InstructionOpcode Ret      = {0x00, 0xC3};          // RET
constexpr InstructionOpcode Add      = {0x00, 0x01};          // ADD (register/memory)
constexpr InstructionOpcode Addss    = {0xF3, 0x0F, 0x58, true};    // ADDSS (SSE)
constexpr InstructionOpcode Addsd    = {0xF2, 0x0F, 0x58, true};    // ADDSD (SSE2)
constexpr InstructionOpcode Sub      = {0x00, 0x29};          // SUB
constexpr InstructionOpcode Subss    = {0xF3, 0x0F, 0x5C, true};    // SUBSS (SSE)
constexpr InstructionOpcode Subsd    = {0xF2, 0x0F, 0x5C, true};    // SUBSD (SSE2)
constexpr InstructionOpcode Imul     = {0x00, 0x69};          // IMUL (with immediate operand)
constexpr InstructionOpcode Mulss    = {0xF3, 0x0F, 0x59, true};    // MULSS (SSE)
constexpr InstructionOpcode Mulsd    = {0xF2, 0x0F, 0x59, true};    // MULSD (SSE2)
constexpr InstructionOpcode Push     = {0x00, 0x50};          // PUSH (register)
constexpr InstructionOpcode Pop      = {0x00, 0x58};          // POP (register)
constexpr InstructionOpcode Mov      = {0x00, 0x89};          // MOV (register/memory)
constexpr InstructionOpcode Movss    = {0xF3, 0x0F, 0x10, true};    // MOVSS (SSE)
constexpr InstructionOpcode Movsd    = {0xF2, 0x0F, 0x10, true};    // MOVSD (SSE2)
constexpr InstructionOpcode Movzx    = {0x00, 0x0F, 0xB6};    // MOVZX
constexpr InstructionOpcode Jmp      = {0x00, 0xE9};          // JMP (near jump)
constexpr InstructionOpcode Cmp      = {0x00, 0x3D};          // CMP (immediate to register/memory)
constexpr InstructionOpcode Ucomiss  = {0x00, 0x0F, 0x2E, true};    // UCOMISS (SSE comparison)
constexpr InstructionOpcode Ucomisd  = {0x66, 0x0F, 0x2E, true};    // UCOMISD (SSE2 comparison)
constexpr InstructionOpcode Sete     = {0x00, 0x0F, 0x94};    // SETE (set if equal)
constexpr InstructionOpcode Setne    = {0x00, 0x0F, 0x95};    // SETNE (set if not equal)
constexpr InstructionOpcode Setge    = {0x00, 0x0F, 0x9D};    // SETGE (set if greater or equal)
constexpr InstructionOpcode Setle    = {0x00, 0x0F, 0x9E};    // SETLE (set if less or equal)
constexpr InstructionOpcode Je       = {0x00, 0x0F, 0x84};    // JE (jump if equal)
constexpr InstructionOpcode Call     = {0x00, 0xE8};          // CALL (near call)
constexpr InstructionOpcode Movdqu   = {0x00, 0x0F, 0x6F};    // MOVDQU (SSE, unaligned load)
constexpr InstructionOpcode Lea      = {0x00, 0x8D};          // LEA (load effective address)
constexpr InstructionOpcode Movsq    = {0x00, 0x0F, 0xD6};    // MOVSQ (SSE2, unaligned load)
constexpr InstructionOpcode Movsb    = {0x00, 0xA4};          // MOVS (SSE2, unaligned load)

static const std::map<InstructionOpcode, std::string> opcodeMap = {
    {Nop, "nop"},
    {Ret, "ret"},
    {Add, "add"},
    {Addss, "addss"},
    {Addsd, "addsd"},
    {Sub, "sub"},
    {Subss, "subss"},
    {Subsd, "subsd"},
    {Imul, "imul"},
    {Mulss, "mulss"},
    {Mulsd, "mulsd"},
    {Push, "push"},
    {Pop, "pop"},
    {Mov, "mov"},
    {Movss, "movss"},
    {Movsd, "movsd"},
    {Movzx, "movzx"},
    {Jmp, "jmp"},
    {Cmp, "cmp"},
    {Ucomiss, "ucomiss"},
    {Ucomisd, "ucomisd"},
    {Sete, "sete"},
    {Setne, "setne"},
    {Setge, "setge"},
    {Setle, "setle"},
    {Je, "je"},
    {Call, "call"},
    {Movdqu, "movdqu"},
    {Lea, "lea"},
    {Movsq, "movsq"},
    {Movsb, "movsb"},
};

static const std::unordered_map<uint8_t, std::string> s_prefixes = {
    {0xF3, "rep"},
};

struct Instruction : CodeGen::Instruction {

    InstructionOpcode m_opcode = {0x00, 0x00};

};

}