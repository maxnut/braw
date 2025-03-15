#include "move-resolver.hpp"
#include "codegen/x86-64/address.hpp"
#include "codegen/x86-64/immediate.hpp"
#include "codegen/x86-64/instruction.hpp"
#include "codegen/x86-64/olabel.hpp"
#include "codegen/x86-64/register.hpp"
#include <cstdint>
#include <memory>
#include <unordered_map>
#include <unordered_set>

namespace CodeGen::x86_64 {

template <typename T>
std::shared_ptr<T> cast(const std::shared_ptr<void>& ptr) {
    return std::static_pointer_cast<T>(ptr);
}

bool operandEquals(std::shared_ptr<Operand> op1, std::shared_ptr<Operand> op2) {
    if(!op1 || !op2 || op1->m_type != op2->m_type)
        return false;

    switch(op1->m_type) {
        case Operand::Type::Register:
            return cast<Operands::Register>(op1)->m_group == cast<Operands::Register>(op2)->m_group;
        case Operand::Type::Address: {
            auto addr1 = cast<Operands::Address>(op1);
            auto addr2 = cast<Operands::Address>(op2);
            return operandEquals(addr1->m_base, addr2->m_base)
                && addr1->m_offset == addr2->m_offset
                && addr1->m_scale == addr2->m_scale
                && operandEquals(addr1->m_index, addr2->m_index);
        }
        case Operand::Type::Immediate:
            return cast<Operands::Immediate>(op1)->m_value == cast<Operands::Immediate>(op2)->m_value;
        case Operand::Type::Label:
            return cast<Operands::Label>(op1)->m_id == cast<Operands::Label>(op2)->m_id;
        default:
            break;
    }

    return false;
}

uint64_t MoveResolver::countOperand(std::shared_ptr<Operand> op, const std::vector<Instruction>& moves) {
    uint64_t counter = 0;
    for(const Instruction& move : moves) {
        if(operandEquals(move.m_operands.at(0), op))
            counter++;
        if(operandEquals(move.m_operands.at(1), op))
            counter++;
    }
    return counter;
}

const Instruction* findForGraph(std::shared_ptr<Operand> op, const std::vector<Instruction>& moves, std::unordered_set<Instruction>& visited) {
    for(size_t i = moves.size(); i >= 0; i--) {
        const Instruction& move = moves[i];
        if(!visited.contains(move) && operandEquals(op, move.m_operands.at(1))) {
            visited.insert(move);
            return &moves[i];
        }
    }
    return nullptr;
}

std::unique_ptr<MoveNode> buildGraph(const std::vector<Instruction>& moves) {
    auto root = std::make_unique<MoveNode>();
    root->m_next = std::make_unique<MoveNode>();
    root->m_operand = moves.back().m_operands.at(1);
    root->m_next->m_operand = moves.back().m_operands.at(0);
    MoveNode* prev = root->m_next.get();

    std::unordered_set<Instruction> visited;
    while(true) {
        auto found = findForGraph(prev->m_operand, moves, visited);
        if(!found)
            break;
        prev->m_next = std::make_unique<MoveNode>();
        prev->m_next->m_operand = found->m_operands.at(0);
        prev = prev->m_next.get();
    }

    return root;    
}

}