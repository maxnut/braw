#include "peephole.hpp"
#include "codegen/x86-64/immediate.hpp"
#include "codegen/x86-64/instruction.hpp"
#include "codegen/x86-64/move-resolver/move_resolver.hpp"
#include "codegen/x86-64/register.hpp"

namespace CodeGen::x86_64 {

std::span<Instruction> matchVainPushPop(InstructionCursor cursor) {
    auto beg = cursor.current();
    if(cursor.get().value().m_opcode != Opcode::Push || !cursor.hasNext()) return {};
    auto pushReg = ((Operands::Register*)cursor.value().m_operands.at(0).get())->m_group;
    if(cursor.next().get().value().m_opcode != Opcode::Pop) return {};
    auto popReg = ((Operands::Register*)cursor.value().m_operands.at(0).get())->m_group;
    if(pushReg != popReg) return {};
    return std::span<Instruction>(beg, cursor.current());
}

std::span<Instruction> matchVainSubAdd(InstructionCursor cursor) {
    auto beg = cursor.current();
    if(cursor.get().value().m_opcode != Opcode::Sub || !cursor.hasNext()) return {};
    auto op1 = cursor.value().m_operands.at(0);
    int amt1 = ((Operands::Immediate*)cursor.value().m_operands.at(1).get())->m_value;
    if(cursor.next().get().value().m_opcode != Opcode::Add) return {};
    auto op2 = cursor.value().m_operands.at(0);
    int amt2 = ((Operands::Immediate*)cursor.value().m_operands.at(1).get())->m_value;
    if(MoveResolver::operandEquals(op1, op2) && amt1 == amt2)
        return std::span<Instruction>(beg, cursor.current());
    return {};
}

std::span<Instruction> matchVainAddSub(InstructionCursor cursor) {
    auto beg = cursor.current();
    if(cursor.get().value().m_opcode != Opcode::Add || !cursor.hasNext()) return {};
    auto op1 = cursor.value().m_operands.at(0);
    int amt1 = ((Operands::Immediate*)cursor.value().m_operands.at(1).get())->m_value;
    if(cursor.next().get().value().m_opcode != Opcode::Sub) return {};
    auto op2 = cursor.value().m_operands.at(0);
    int amt2 = ((Operands::Immediate*)cursor.value().m_operands.at(1).get())->m_value;
    if(MoveResolver::operandEquals(op1, op2) && amt1 == amt2)
        return std::span<Instruction>(beg, cursor.current() + 1);
    return {};
}

std::span<Instruction> matchVainMov(InstructionCursor cursor) {
    auto beg = cursor.current();
    auto i = cursor.get().value();
    if((i.m_opcode == Mov || i.m_opcode == Movss || i.m_opcode == Movsd) && MoveResolver::operandEquals(i.m_operands.at(0), i.m_operands.at(1)))
        return std::span<Instruction>(beg, cursor.current() + 1);
    return {};
}

std::span<Instruction> matchOverwrite(InstructionCursor cursor) {
    auto beg = cursor.current();
    if(cursor.get().value().m_opcode != Opcode::Mov || !cursor.hasNext()) return {};
    auto op1 = cursor.value().m_operands.at(0);
    if(cursor.next().get().value().m_opcode != Opcode::Mov) return {};
    auto op2 = cursor.value().m_operands.at(0);
    if(MoveResolver::operandEquals(op1, op2))
        return std::span<Instruction>(beg, cursor.current() + 1);
    return {};
}

std::vector<Instruction> replaceEmpty(std::span<Instruction> span) {
    return {};
}

std::vector<Instruction> replaceOverwrite(std::span<Instruction> span) {
    std::vector<Instruction> ret;
    ret.push_back(span[1]);
    return ret;
}

bool Peephole::run(File& file) {
    std::vector<std::pair<MatcherFunction, ReplaceFunction>> rules = {
        {matchVainPushPop, replaceEmpty},
        {matchVainSubAdd, replaceEmpty},
        {matchVainAddSub, replaceEmpty},
        {matchVainMov, replaceEmpty},
        {matchOverwrite, replaceOverwrite}
    };
    InstructionCursor cursor(file.m_text.m_instructions.begin(), file.m_text.m_instructions.end());
    bool change = false;

    while(cursor.hasNext()) {
        for(auto& rule : rules) {
            auto span = rule.first(cursor);
            if(span.empty()) continue;
            auto vec = rule.second(span);
            file.m_text.m_instructions.erase(span.begin(), span.end());
            file.m_text.m_instructions.insert(span.begin(), vec.begin(), vec.end());
            change = true;
            if(cursor.hasPrev()) cursor.prev();
        }
        cursor.next();
    }
    return change;
}

}