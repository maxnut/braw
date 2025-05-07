#include "cse.hpp"
#include "ssa/instruction.hpp"
#include "ssa/operand.hpp"
#include <iostream>
#include <memory>
#include <unordered_map>

namespace SSA {

bool CSE::run(Function& f) {
    std::unordered_set<std::shared_ptr<Block>> visited;
    std::unordered_map<size_t, std::shared_ptr<Register>> expressions;
    std::unordered_map<std::string, std::shared_ptr<Register>> originalToVersioned;
    return run(f, f.m_blocks.at(0), visited, expressions, originalToVersioned);
}

bool CSE::run(Function& f, std::shared_ptr<Block> block, std::unordered_set<std::shared_ptr<Block>>& visited, std::unordered_map<size_t, std::shared_ptr<Register>>& expressions, std::unordered_map<std::string, std::shared_ptr<Register>>& originalToVersioned) {
    if(visited.contains(block))
        return false;
    visited.insert(block);
    bool change = false;

    for(size_t i = block->m_instructionRange.first; i <= block->m_instructionRange.second; i++) {
        auto ins = f.m_instructions.at(i).get();
        if(ins->m_type != Instruction::Assign) continue;
        auto ass = (Assignment*)ins;
        if(ass->m_to->m_type != Operand::Register || !ass->m_operation->m_memory || ass->m_operation->m_memory->m_id.contains("phi")) continue;
        if((ass->m_operation->m_o1->m_type == Operand::Register && std::static_pointer_cast<Register>(ass->m_operation->m_o1)->m_id.contains("phi")) ||
            (ass->m_operation->m_o2 && ass->m_operation->m_o2->m_type == Operand::Register && std::static_pointer_cast<Register>(ass->m_operation->m_o2)->m_id.contains("phi"))) continue;

        auto reg = std::static_pointer_cast<Register>(ass->m_to);
        originalToVersioned[reg->m_originalId] = reg;
        switch (ass->m_operation->m_type) {
            case Operation::Add:
            case Operation::Subtract:
            case Operation::Multiply:
            case Operation::Divide:
            case Operation::CompareEquals:
            case Operation::CompareNotEquals:
            case Operation::CompareGreaterEquals:
            case Operation::CompareLessEquals:
            case Operation::CompareGreater:
            case Operation::CompareLess:
            case Operation::Modulo:
            case Operation::And:
            case Operation::Or:
            case Operation::Xor:
            case Operation::Point:
            case Operation::Dereference:
            case Operation::PartialDereference:
            case Operation::Upsize:
            case Operation::Downsize:
            case Operation::LogicalNot: {
                size_t hash = hashOperation(ass->m_operation.get());
                if(!expressions.contains(hash)) {
                    expressions[hash] = reg;
                    break;
                }
                auto newOp = expressions.at(hash);
                auto upToDate = originalToVersioned.at(newOp->m_originalId);
                if(hashOperand(upToDate.get()) != hashOperand(newOp.get()))
                    break;
                auto newOper = std::make_shared<Operation>(Operation::Load, newOp->m_typeInfo, newOp, ass->m_operation->m_memory);
                ass->m_operation = newOper;
                change = true;
                break;
            }
            case Operation::Load:
            case Operation::Reference:
                break;
        }
    }

    for(auto d : block->m_dominated)
        change |= run(f, d, visited, expressions, originalToVersioned);

    return change;
}

bool isCommutative(Operation::Type t) {
    return t == Operation::Add || t == Operation::Multiply ||
           t == Operation::And || t == Operation::Or ||
           t == Operation::Xor || t == Operation::CompareEquals ||
           t == Operation::CompareNotEquals;
}

size_t CSE::hashOperation(const Operation* op) {
    size_t h = std::hash<int>{}(op->m_type);

    size_t h1 = hashOperand(op->m_o1.get());
    size_t h2 = hashOperand(op->m_o2.get());

    if (isCommutative(op->m_type))
        if (h1 > h2) std::swap(h1, h2);

    h ^= h1 + 0x9e3779b9 + (h << 6) + (h >> 2);
    h ^= h2 + 0x9e3779b9 + (h << 6) + (h >> 2);

    if(op->m_memory) {
        size_t h3 = hashOperand(op->m_memory.get());
        h ^= h3 + 0x9e3779b9 + (h << 6) + (h >> 2);
    }

    return h;
}

size_t CSE::hashOperand(const Operand* op) {
    if (!op) return 0;

    size_t h = std::hash<int>{}(op->m_type);
    h ^= std::hash<size_t>{}(op->m_scale) + 0x9e3779b9 + (h << 6) + (h >> 2);

    if (op->m_type == Operand::Register) {
        auto* reg = static_cast<const Register*>(op);
        h ^= std::hash<std::string>{}(reg->m_id) + 0x9e3779b9 + (h << 6) + (h >> 2);
    } else if (op->m_type == Operand::Immediate) {
        auto* imm = static_cast<const Immediate*>(op);
        h ^= std::visit([](auto&& v) {
            return std::hash<std::decay_t<decltype(v)>>{}(v);
        }, imm->m_value);
    }

    return h;
}

}