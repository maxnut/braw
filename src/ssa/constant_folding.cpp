#include "constant_folding.hpp"
#include "ssa/instruction.hpp"
#include "ssa/operand.hpp"
#include "ssa/operation.hpp"
#include <cstdint>
#include <memory>
#include <variant>

namespace SSA {

bool ConstantFolding::run(SSA::Function& fun) {
    bool change = false;
    for(auto& instr : fun.m_instructions) {
        switch (instr->m_type) {
        case Instruction::Assign: {
            auto assign = (Assignment*)instr.get();
            Operation* op = assign->m_operation.get();
            if(op->m_o1->m_type != Operand::Immediate || (op->m_o2 && op->m_o2->m_type != Operand::Immediate)) continue;
            auto imm = fold(assign->m_operation.get());
            if(!imm) break;
            assign->m_operation = std::make_shared<Operation>(Operation::Load, imm->m_typeInfo, imm);
            change = true;
            break;
        }
        case Instruction::Allocate:
        case Instruction::Call:
        case Instruction::Return:
        case Instruction::JumpFalse:
        case Instruction::JumpTrue:
        case Instruction::Jump:
        case Instruction::Label:
        case Instruction::Phi:
        case Instruction::WriteMem:
            break;
        }
    }
    return change;
}

std::shared_ptr<Immediate> ConstantFolding::fold(const SSA::Operation* op) {
    auto imm1 = (Immediate*)op->m_o1.get();
    auto imm2 = op->m_o2 ? (Immediate*)op->m_o2.get() : nullptr;
    auto ret = std::make_shared<Immediate>();
    ret->m_typeInfo = imm1->m_typeInfo;
    switch (op->m_type) {
        case Operation::Add: {
            switch(imm1->m_value.index()) {
                case 0: ret->m_value = std::get<int>(imm1->m_value) + std::get<int>(imm2->m_value); break;
                case 1: ret->m_value = std::get<long>(imm1->m_value) + std::get<long>(imm2->m_value); break;
                case 2: ret->m_value = std::get<float>(imm1->m_value) + std::get<float>(imm2->m_value); break;
                case 3: ret->m_value = std::get<double>(imm1->m_value) + std::get<double>(imm2->m_value); break;
                case 4: ret->m_value = std::get<bool>(imm1->m_value) + std::get<bool>(imm2->m_value); break;
                case 6: ret->m_value = std::get<char>(imm1->m_value) + std::get<char>(imm2->m_value); break;
                case 7: ret->m_value = std::get<uint32_t>(imm1->m_value) + std::get<uint32_t>(imm2->m_value); break;
                case 8: ret->m_value = std::get<uint64_t>(imm1->m_value) + std::get<uint64_t>(imm2->m_value); break;
                default: break;
            }
            break;
        }
        case Operation::Subtract: {
            switch(imm1->m_value.index()) {
                case 0: ret->m_value = std::get<int>(imm1->m_value) - std::get<int>(imm2->m_value); break;
                case 1: ret->m_value = std::get<long>(imm1->m_value) - std::get<long>(imm2->m_value); break;
                case 2: ret->m_value = std::get<float>(imm1->m_value) - std::get<float>(imm2->m_value); break;
                case 3: ret->m_value = std::get<double>(imm1->m_value) - std::get<double>(imm2->m_value); break;
                case 4: ret->m_value = std::get<bool>(imm1->m_value) - std::get<bool>(imm2->m_value); break;
                case 6: ret->m_value = std::get<char>(imm1->m_value) - std::get<char>(imm2->m_value); break;
                case 7: ret->m_value = std::get<uint32_t>(imm1->m_value) - std::get<uint32_t>(imm2->m_value); break;
                case 8: ret->m_value = std::get<uint64_t>(imm1->m_value) - std::get<uint64_t>(imm2->m_value); break;
                default: break;
            }
            break;
        }
        case Operation::Multiply: {
            switch(imm1->m_value.index()) {
                case 0: ret->m_value = std::get<int>(imm1->m_value) * std::get<int>(imm2->m_value); break;
                case 1: ret->m_value = std::get<long>(imm1->m_value) * std::get<long>(imm2->m_value); break;
                case 2: ret->m_value = std::get<float>(imm1->m_value) * std::get<float>(imm2->m_value); break;
                case 3: ret->m_value = std::get<double>(imm1->m_value) * std::get<double>(imm2->m_value); break;
                case 4: ret->m_value = std::get<bool>(imm1->m_value) * std::get<bool>(imm2->m_value); break;
                case 6: ret->m_value = std::get<char>(imm1->m_value) * std::get<char>(imm2->m_value); break;
                case 7: ret->m_value = std::get<uint32_t>(imm1->m_value) * std::get<uint32_t>(imm2->m_value); break;
                case 8: ret->m_value = std::get<uint64_t>(imm1->m_value) * std::get<uint64_t>(imm2->m_value); break;
                default: break;
            }
            break;
        }
        case Operation::Divide: {
            switch(imm1->m_value.index()) {
                case 0: ret->m_value = std::get<int>(imm1->m_value) / std::get<int>(imm2->m_value); break;
                case 1: ret->m_value = std::get<long>(imm1->m_value) / std::get<long>(imm2->m_value); break;
                case 2: ret->m_value = std::get<float>(imm1->m_value) / std::get<float>(imm2->m_value); break;
                case 3: ret->m_value = std::get<double>(imm1->m_value) / std::get<double>(imm2->m_value); break;
                case 4: ret->m_value = std::get<bool>(imm1->m_value) / std::get<bool>(imm2->m_value); break;
                case 6: ret->m_value = std::get<char>(imm1->m_value) / std::get<char>(imm2->m_value); break;
                case 7: ret->m_value = std::get<uint32_t>(imm1->m_value) / std::get<uint32_t>(imm2->m_value); break;
                case 8: ret->m_value = std::get<uint64_t>(imm1->m_value) / std::get<uint64_t>(imm2->m_value); break;
                default: break;
            }
            break;
        }
        case Operation::Modulo: {
            switch(imm1->m_value.index()) {
                case 0: ret->m_value = std::get<int>(imm1->m_value) % std::get<int>(imm2->m_value); break;
                case 1: ret->m_value = std::get<long>(imm1->m_value) % std::get<long>(imm2->m_value); break;
                case 4: ret->m_value = std::get<bool>(imm1->m_value) % std::get<bool>(imm2->m_value); break;
                case 6: ret->m_value = std::get<char>(imm1->m_value) % std::get<char>(imm2->m_value); break;
                case 7: ret->m_value = std::get<uint32_t>(imm1->m_value) % std::get<uint32_t>(imm2->m_value); break;
                case 8: ret->m_value = std::get<uint64_t>(imm1->m_value) % std::get<uint64_t>(imm2->m_value); break;
                default: break;
            }
            break;
        }
        case Operation::And: {
            switch(imm1->m_value.index()) {
                case 0: ret->m_value = std::get<int>(imm1->m_value) & std::get<int>(imm2->m_value); break;
                case 1: ret->m_value = std::get<long>(imm1->m_value) & std::get<long>(imm2->m_value); break;
                case 4: ret->m_value = std::get<bool>(imm1->m_value) & std::get<bool>(imm2->m_value); break;
                case 6: ret->m_value = std::get<char>(imm1->m_value) & std::get<char>(imm2->m_value); break;
                case 7: ret->m_value = std::get<uint32_t>(imm1->m_value) & std::get<uint32_t>(imm2->m_value); break;
                case 8: ret->m_value = std::get<uint64_t>(imm1->m_value) & std::get<uint64_t>(imm2->m_value); break;
                default: break;
            }
            break;
        }
        case Operation::Or: {
            switch(imm1->m_value.index()) {
                case 0: ret->m_value = std::get<int>(imm1->m_value) | std::get<int>(imm2->m_value); break;
                case 1: ret->m_value = std::get<long>(imm1->m_value) | std::get<long>(imm2->m_value); break;
                case 4: ret->m_value = std::get<bool>(imm1->m_value) | std::get<bool>(imm2->m_value); break;
                case 6: ret->m_value = std::get<char>(imm1->m_value) | std::get<char>(imm2->m_value); break;
                case 7: ret->m_value = std::get<uint32_t>(imm1->m_value) | std::get<uint32_t>(imm2->m_value); break;
                case 8: ret->m_value = std::get<uint64_t>(imm1->m_value) | std::get<uint64_t>(imm2->m_value); break;
                default: break;
            }
            break;
        }
        case Operation::Xor: {
            switch(imm1->m_value.index()) {
                case 0: ret->m_value = std::get<int>(imm1->m_value) ^ std::get<int>(imm2->m_value); break;
                case 1: ret->m_value = std::get<long>(imm1->m_value) ^ std::get<long>(imm2->m_value); break;
                case 4: ret->m_value = std::get<bool>(imm1->m_value) ^ std::get<bool>(imm2->m_value); break;
                case 6: ret->m_value = std::get<char>(imm1->m_value) ^ std::get<char>(imm2->m_value); break;
                case 7: ret->m_value = std::get<uint32_t>(imm1->m_value) ^ std::get<uint32_t>(imm2->m_value); break;
                case 8: ret->m_value = std::get<uint64_t>(imm1->m_value) ^ std::get<uint64_t>(imm2->m_value); break;
                default: break;
            }
            break;
        }
        case Operation::LogicalNot: {
            switch(imm1->m_value.index()) {
                case 0: ret->m_value = !std::get<int>(imm1->m_value); break;
                case 1: ret->m_value = !std::get<long>(imm1->m_value); break;
                case 4: ret->m_value = !std::get<bool>(imm1->m_value); break;
                case 6: ret->m_value = !std::get<char>(imm1->m_value); break;
                case 7: ret->m_value = !std::get<uint32_t>(imm1->m_value); break;
                case 8: ret->m_value = !std::get<uint64_t>(imm1->m_value); break;
                default: break;
            }
            break;
        }
        case Operation::Point:
        case Operation::Dereference:
        case Operation::PartialDereference:
        case Operation::Upsize:
        case Operation::Downsize:
        case Operation::CompareEquals:
        case Operation::CompareNotEquals:
        case Operation::CompareGreaterEquals:
        case Operation::CompareLessEquals:
        case Operation::CompareGreater:
        case Operation::CompareLess:
        case Operation::Load:
        case Operation::Reference:
            return nullptr;
        }
        return ret;
    }
}