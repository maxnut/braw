#include "move-resolver.hpp"
#include "codegen/x86-64/code_generator.hpp"
#include "codegen/x86-64/address.hpp"
#include "codegen/x86-64/immediate.hpp"
#include "codegen/x86-64/instruction.hpp"
#include "codegen/x86-64/olabel.hpp"
#include "codegen/x86-64/register.hpp"
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

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

void poop(const MoveNode* node) {
    std::cout << cast<Operands::Register>(node->m_operand)->m_ids[Operand::Size::Qword];
    if(node->m_next) {
        std::cout << " -> ";
        poop(node->m_next.get());
    }
}

const int findForSwap(std::shared_ptr<Operand> op, const std::vector<Instruction>& moves, size_t off) {
    for(size_t i = off; i < moves.size(); i++) {
        const Instruction& move = moves[i];
        if(operandEquals(op, move.m_operands.at(1))) {
            return i;
        }
    }
    return -1;
}

const Instruction* findForGraph(std::shared_ptr<Operand> op, const std::vector<Instruction>& moves, std::unordered_set<int>& visited) {
    for(int i = moves.size() - 1; i >= 0; i--) {
        const Instruction& move = moves[i];
        if(!visited.contains(i) && operandEquals(op, move.m_operands.at(1))) {
            visited.insert(i);
            return &moves[i];
        }
    }
    return nullptr;
}

const void replaceForGraph(std::shared_ptr<Operand> op, std::shared_ptr<Operand> replace, std::vector<Instruction>& moves) {
    for(int i = moves.size() - 1; i >= 0; i--) {
        Instruction& move = moves[i];
        if(operandEquals(op, move.m_operands.at(1))) {
            move.m_operands[1] = replace;
        }
    }
}

std::vector<Instruction> MoveResolver::resolve(std::vector<Instruction> from, CodeGenerator& codegen, FunctionContext& ctx) {
    std::vector<Instruction> result;
    std::erase_if(from, [from, &result](Instruction& instr){
        if(instr.m_opcode != Mov) {
            result.push_back(instr);
            return true;
        }
        return false;
    });
    std::erase_if(from, [from, &result](Instruction& instr){
        if(operandEquals(instr.m_operands.at(0), instr.m_operands.at(1)) || (countOperand(instr.m_operands.at(0), from) && countOperand(instr.m_operands.at(1), from) == 1)) {
            result.push_back(instr);
            return true;
        }
        return false;
    });

    if(from.empty())
        return result;

    std::vector<std::shared_ptr<Operands::Register>> pushed;
    std::shared_ptr<Operands::Register> loop = nullptr;
    while(true) {
        std::unique_ptr<MoveNode> graphRoot = buildGraph(from);
        // poop(graphRoot.get());
        // std::cout << std::endl;
        loop = cast<Operands::Register>(findLoop(graphRoot.get()));
        if(!loop)
            break;
        ctx.m_spills += loop->m_typeInfo.m_size;
        codegen.push(loop, ctx);
        pushed.push_back(loop);
        replaceForGraph(loop, std::make_shared<Operands::Address>(codegen.m_registers.at(Operands::Register::RBP), -ctx.m_spills, loop->m_typeInfo), from);
    };

    while(true) {
        bool swapped = false;
        for(size_t i = 0; i < from.size(); i++) {
            Instruction& move = from[i];
            int idx = findForSwap(move.m_operands.at(0), from, i + 1);
            if(idx == -1)
                continue;
            std::swap(from[i], from[idx]);
            swapped = true;
            break;
        }
        if(!swapped)
            break;
    }

    for(Instruction& instr : from) {
        if(operandEquals(instr.m_operands.at(0), instr.m_operands.at(1)))
            continue;
        result.push_back(instr);
    }
    
    std::reverse(pushed.begin(), pushed.end());
    for(auto reg : pushed) {
        codegen.pop(reg, ctx);
        ctx.m_spills -= reg->m_typeInfo.m_size;
    }
    return result;
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

std::unique_ptr<MoveNode> MoveResolver::buildGraph(const std::vector<Instruction>& moves) {
    auto root = std::make_unique<MoveNode>();
    root->m_next = std::make_unique<MoveNode>();
    root->m_operand = moves.back().m_operands.at(1);
    root->m_next->m_operand = moves.back().m_operands.at(0);
    MoveNode* prev = root->m_next.get();

    std::unordered_set<int> visited;
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

std::shared_ptr<Operand> MoveResolver::findLoop(const MoveNode* root) {
    std::unordered_set<Operands::Register::RegisterGroup> visited;
    const MoveNode* next = root;
    while(next) {
        if(visited.contains(cast<Operands::Register>(next->m_operand)->m_group))
            return next->m_operand;
        visited.insert(cast<Operands::Register>(next->m_operand)->m_group);
        next = next->m_next.get();
    }
    return nullptr;
}

}