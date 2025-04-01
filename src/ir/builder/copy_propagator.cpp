#include "copy_propagator.hpp"
#include "ir/builder/ir_builder.hpp"
#include "ir/instruction.hpp"
#include "ir/instructions/basic.hpp"
#include "ir/instructions/call.hpp"
#include "ir/operand.hpp"
#include <vector>

bool isJump(const Instruction* i) {
    return i->m_type == Instruction::Jump || i->m_type == Instruction::JumpFalse || i->m_type == Instruction::JumpTrue;
}

void CopyPropagator::propagate(Function& f) {
    bool removed = false;
    do {
        removed = false;
        auto points = getModificationPoints(f);
        std::vector<Block> blocks = getBlocks(f);
        for(Block& block : blocks) {
            //skip last
            for(size_t i = block.m_instructionRange.first; i < block.m_instructionRange.second; i++) {
                if(f.m_instructions[i]->m_type != Instruction::Move)
                    continue;
                auto basic = (BasicInstruction*)f.m_instructions[i].get();
                if(basic->m_o1.index() != 1 || (basic->m_o2.index() != 1 && basic->m_o2.index() != 3))
                    continue;

                if(basic->m_o2.index() == 3) // TODO: remove this when modification point fetching works with addresses
                    continue;
                    
                auto reg = std::get<1>(((BasicInstruction*)f.m_instructions[i].get())->m_o1);
                if(replace(f, points.at(reg->m_id), i + 1, block.m_instructionRange.second, reg->m_id, basic->m_o2)) {
                    removed = true;
                    f.m_instructions.erase(f.m_instructions.begin() + i);
                    break;
                }
            }
            if(removed)
                break;
        }
    } while(removed);
}

std::vector<Block> CopyPropagator::getBlocks(const Function& f) {
    std::vector<Block> blocks;
    Block* current = nullptr;

    for(size_t i = 0; i < f.m_instructions.size(); i++) {
        if(f.m_instructions.at(i)->m_type == Instruction::Label || (i > 0 && isJump(f.m_instructions.at(i - 1).get()))) {
            blocks.push_back(Block());
            current = &blocks.at(blocks.size() - 1);
            current->m_instructionRange.first = i;
        }
        current->m_instructionRange.second = i;
    }
    return blocks;
}

std::unordered_map<std::string, std::unordered_set<size_t>> CopyPropagator::getModificationPoints(const Function& f) {
    std::unordered_map<std::string, std::unordered_set<size_t>> result;
    for(size_t i = 0; i < f.m_instructions.size(); i++) {
        auto& inst = f.m_instructions.at(i);
        switch(inst->m_type) {
            default: {
                BasicInstruction* basic = (BasicInstruction*)inst.get();
                if(basic->m_o1.index() == 1) {
                    result[std::get<1>(basic->m_o1)->m_id].insert(i);
                    break;
                }
                else if(basic->m_o1.index() == 3) {
                    auto addr = std::get<Address>(basic->m_o1);
                    result[addr.m_base->m_id].insert(i);
                    break;
                }
                break;
            }
            case Instruction::Label:
            case Instruction::Call:
            case Instruction::Return:
                break;
        }
    }
    return result;
}

bool CopyPropagator::replace(Function& f, const std::unordered_set<size_t>& points, size_t from, size_t orBreak, const std::string& replaceId, Operand replaceWith) {
    bool didReplace = false;
    auto doReplace = [&](Operand* replace) {
        if(replace->index() == 1 && std::get<1>(*replace)->m_id == replaceId) {
            *replace = replaceWith;
            didReplace = true;
        }
        else if(replace->index() == 3 && replaceWith.index() == 1) {
            auto reg = std::get<1>(replaceWith);
            Address* addr = std::get_if<Address>(replace);
            if(addr->m_base->m_id == replaceId) {
                addr->m_base = reg;
                didReplace = true;
            }
            if(addr->m_index && addr->m_index->m_id == replaceId) {
                addr->m_index = reg;
                didReplace = true;
            }
        }
    };

    for(;from < orBreak; from++) {
        if(points.contains(from))
            break;
        switch(f.m_instructions[from]->m_type) {
            default: {
                BasicInstruction* basic = (BasicInstruction*)f.m_instructions[from].get();
                doReplace(&basic->m_o1);
                doReplace(&basic->m_o2);
                doReplace(&basic->m_o3);
                doReplace(&basic->m_o4);
                break;
            }
            case Instruction::Call: {
                CallInstruction* call = (CallInstruction*)f.m_instructions[from].get();
                doReplace((Operand*)&call->m_optReturn);
                for(auto& param : call->m_parameters) {
                    doReplace(&param);
                }
                break;
            }
            case Instruction::Label:
            case Instruction::Return:
                break;
        }
    }
    return didReplace;
}