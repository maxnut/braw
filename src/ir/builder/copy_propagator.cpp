#include "copy_propagator.hpp"
#include "ir/instruction.hpp"
#include "ir/instructions/basic.hpp"
#include "ir/instructions/call.hpp"
#include "ir/operand.hpp"
#include <memory>
#include <unordered_map>
#include <vector>

bool isJump(const Instruction* i) {
    return i->m_type == Instruction::Jump || i->m_type == Instruction::JumpFalse || i->m_type == Instruction::JumpTrue;
}

void CopyPropagator::propagate(Function& f) {
    bool removed = false;
    do {
        removed = false;
        auto points = getModificationPoints(f);
        std::vector<std::shared_ptr<Block>> blocks = getBlocks(f);
        for(auto& block : blocks) {
            //skip last
            for(size_t i = block->m_instructionRange.first; i < block->m_instructionRange.second; i++) {
                if(f.m_instructions[i]->m_type != Instruction::Move)
                    continue;
                auto basic = (BasicInstruction*)f.m_instructions[i].get();
                if(basic->m_o1.index() != 1 || (basic->m_o2.index() != 1 && basic->m_o2.index() != 3))
                    continue;

                if(basic->m_o2.index() == 3) // TODO: remove this when modification point fetching works with addresses
                    continue;
                    
                auto reg = std::get<1>(basic->m_o1);
                auto reg2 = std::get<1>(basic->m_o2);
                std::unordered_set<std::shared_ptr<Block>> visited;
                auto pointsMerged = points.at(reg->m_id);
                if(points.contains(reg2->m_id))
                    pointsMerged.insert(points.at(reg2->m_id).begin(), points.at(reg2->m_id).end());
                
                if(f.m_retains.contains(reg->m_id) || checkReassign(f, pointsMerged, i + 1, block, visited))
                    continue;
                visited.clear();
                
                if(replace(f, i + 1, block, reg->m_id, basic->m_o2, visited)) {
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

std::vector<std::shared_ptr<Block>> CopyPropagator::getBlocks(const Function& f) {
    std::vector<std::shared_ptr<Block>> blocks;
    Block* current = nullptr;
    std::unordered_map<size_t, size_t> blockForInstruction;

    for(size_t i = 0; i < f.m_instructions.size(); i++) {
        if(f.m_instructions.at(i)->m_type == Instruction::Label || (i > 0 && isJump(f.m_instructions.at(i - 1).get()))) {
            blocks.push_back(std::make_shared<Block>());
            current = blocks.at(blocks.size() - 1).get();
            current->m_instructionRange.first = i;
        }
        current->m_instructionRange.second = i;
        blockForInstruction[i] = blocks.size() - 1;
    }
    std::unordered_set<std::shared_ptr<Block>> visited;
    buildGraphRecursive(blocks.at(0), blockForInstruction, blocks, visited, f);
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
            case Instruction::Call: {
                auto call = (CallInstruction*)inst.get();
                if(call->m_optReturn) {
                    result[call->m_optReturn->m_id].insert(i);
                }
                break;
            }
            case Instruction::Label:
            case Instruction::Return:
                break;
        }
    }
    return result;
}

bool CopyPropagator::replace(Function& f, size_t from, std::shared_ptr<Block> block, const std::string& replaceId, Operand replaceWith, std::unordered_set<std::shared_ptr<Block>>& visited) {
    if(visited.contains(block))
        return false;
    visited.insert(block);
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

    for(;from <= block->m_instructionRange.second; from++) {
        switch(f.m_instructions[from]->m_type) {
            default: {
                BasicInstruction* basic = (BasicInstruction*)f.m_instructions[from].get();
                doReplace(&basic->m_o1);
                doReplace(&basic->m_o2);
                doReplace(&basic->m_o3);
                doReplace(&basic->m_o4);
                break;
            }
            case Instruction::PartialDereference: {
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

    for(auto con : block->m_connections)
        didReplace |= replace(f, con->m_instructionRange.first, con, replaceId, replaceWith, visited);
    
    return didReplace;
}


int64_t getJumpTarget(const Instruction* i, const std::vector<std::unique_ptr<Instruction>>& instructions) {
    BasicInstruction* basic = (BasicInstruction*)i;
    std::string label = i->m_type == Instruction::Jump ? std::get<Label>(basic->m_o1).m_id : std::get<Label>(basic->m_o2).m_id;
    for(size_t i = 0; i < instructions.size(); i++) {
        if(instructions.at(i)->m_type != Instruction::Label) 
            continue;
        if(((Label*)instructions.at(i).get())->m_id == label)
            return i;
    }
    return -1;
}

void CopyPropagator::buildGraphRecursive(std::shared_ptr<Block> root, const std::unordered_map<size_t, size_t>& blockForInstruction, const std::vector<std::shared_ptr<Block>>& blocks, std::unordered_set<std::shared_ptr<Block>>& visited, const Function& f) {
    if(visited.contains(root))
        return;
    visited.insert(root);
    auto& lastInstruction = f.m_instructions.at(root->m_instructionRange.second);
    if(lastInstruction->m_type == Instruction::JumpFalse || lastInstruction->m_type == Instruction::JumpTrue || lastInstruction->m_type == Instruction::Jump) {
        std::shared_ptr<Block> next = blocks.at(blockForInstruction.at(getJumpTarget(lastInstruction.get(), f.m_instructions)));
        root->m_connections.push_back(next);
        buildGraphRecursive(next, blockForInstruction, blocks, visited, f);
        if(lastInstruction->m_type == Instruction::Jump)
            return;
    }

    if(blockForInstruction.contains(root->m_instructionRange.second + 1)) {
        std::shared_ptr<Block> next = blocks.at(blockForInstruction.at(root->m_instructionRange.second + 1));
        root->m_connections.push_back(next);
        buildGraphRecursive(next, blockForInstruction, blocks, visited, f);
    }
}

bool CopyPropagator::checkReassign(Function& f, const std::unordered_set<size_t>& points, size_t from, std::shared_ptr<Block> block, std::unordered_set<std::shared_ptr<Block>>& visited) {
    for(; from < block->m_instructionRange.second; from++) {
        if(points.contains(from)) {
            return true;
        }
    }
    if(visited.contains(block))
        return false;
    visited.insert(block);
    for(auto con : block->m_connections) {
        if(checkReassign(f, points, con->m_instructionRange.first, con, visited))
            return true;
    }
    return false;
}