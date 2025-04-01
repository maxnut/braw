#include "liveness_propagator.hpp"
#include "ir/instruction.hpp"
#include "ir/instructions/basic.hpp"
#include "ir/instructions/call.hpp"
#include "ir/operand.hpp"
#include <cstdint>
#include <memory>
#include <unordered_map>

namespace CodeGen::x86_64 {

bool isJump(const Instruction* i) {
    return i->m_type == Instruction::Jump || i->m_type == Instruction::JumpFalse || i->m_type == Instruction::JumpTrue;
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

PropagatorResult Propagator::buildGraph(const Function& f) {
    PropagatorResult result;
    std::shared_ptr<Block> current = nullptr;

    for(size_t i = 0; i < f.m_instructions.size(); i++) {
        if(f.m_instructions.at(i)->m_type == Instruction::Label || (i > 0 && isJump(f.m_instructions.at(i - 1).get()))) {
            result.blocks.push_back(std::make_shared<Block>());
            current = result.blocks.back();
            current->m_instructionRange.first = i;
        }
        current->m_instructionRange.second = i;
        result.blockForInstruction[i] = result.blocks.size() - 1;
    }

    for(auto& block : result.blocks)
        fillRanges(f, block.get());

    std::unordered_set<std::shared_ptr<Block>> visited;
    buildGraphRecursive(result.blocks.at(0), result.blockForInstruction, result.blocks, visited, f);

    visited.clear();
    visit(result.blocks.at(0), visited);

    visited.clear();
    propagate(result.blocks.at(0), visited);
    result.root = result.blocks.at(0);

    return result;
}

void Propagator::buildGraphRecursive(std::shared_ptr<Block> root, const std::unordered_map<size_t, size_t>& blockForInstruction, const std::vector<std::shared_ptr<Block>>& blocks, std::unordered_set<std::shared_ptr<Block>>& visited, const Function& f) {
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

void Propagator::propagate(std::shared_ptr<Block> root, std::unordered_set<std::shared_ptr<Block>>& visited) {
    if(visited.contains(root))
        return;
    visited.insert(root);

    for(auto range : root->m_rangeVector) {
        for(auto conn : root->m_connections) {
            if(!conn->m_ranges.contains(range->m_id))
                continue;

            range->m_range.second = root->m_instructionRange.second;
            conn->m_ranges.at(range->m_id)->m_range.first = conn->m_instructionRange.first;
        }
    }
    
    for(auto conn : root->m_connections)
        propagate(conn, visited);
}

void Propagator::visit(std::shared_ptr<Block> root, std::unordered_set<std::shared_ptr<Block>>& visited) {
    if(visited.contains(root))
        return;
    visited.insert(root);
    std::vector<std::shared_ptr<Block>> path;
    std::unordered_set<std::shared_ptr<Block>> visited2;
    fillHoles(root, root, path, visited2);
    for(auto conn : root->m_connections)
        visit(conn, visited);
}

void Propagator::fillHoles(std::shared_ptr<Block> from, std::shared_ptr<Block> current, std::vector<std::shared_ptr<Block>>& path, std::unordered_set<std::shared_ptr<Block>>& visited) {
    path.push_back(current);

    if(path.size() > 2) {
        for(std::shared_ptr<Range> range : from->m_rangeVector) {
            if(!current->m_ranges.contains(range->m_id))
                continue;

            for(size_t i = 1; i < path.size() - 1; i++) {
                std::shared_ptr<Block> block = path[i];
                if(block->m_ranges.contains(range->m_id))
                    continue;
                auto copy = std::make_shared<Range>();
                copy->m_forceTag = range->m_forceTag;
                copy->m_id = range->m_id;
                copy->m_isPointedOrDereferenced = range->m_isPointedOrDereferenced;
                copy->m_isAssignedFirst = range->m_isAssignedFirst;
                copy->m_registerType = range->m_registerType;
                copy->m_typeInfo = range->m_typeInfo;
                copy->m_scale = range->m_scale;
                copy->m_range = block->m_instructionRange;
                block->m_ranges[range->m_id] = copy;
                block->m_rangeVector.push_back(copy);
            }
        }
    }
    
    if(visited.contains(current)) {
        path.pop_back();
        return;
    }
    visited.insert(current);
    for(auto conn : current->m_connections)
        fillHoles(from, conn, path, visited);
    path.pop_back();
}

void Propagator::fillRanges(const Function& function, Block* result) {
    auto tryRegister = [&](::Operand o, uint32_t i, bool assignmentIfFirst = false, Operands::Register::RegisterGroup forceRegister = Operands::Register::Count) {
        if(o.index() != 1 && o.index() != 3)
            return;

        auto r = o.index() == 3 ? std::get<Address>(o).m_base : std::get<std::shared_ptr<Register>>(o);

        while(r) {
            if(r->m_id == "%return" || r->m_id == "%returnF") // the return register will always be rax/xmm0
                return;

            if(!result->m_ranges.contains(r->m_id)) {
                result->m_ranges[r->m_id] = std::make_shared<Range>();
                result->m_ranges[r->m_id]->m_range.first = i;
                result->m_rangeVector.push_back(result->m_ranges[r->m_id]);
                result->m_ranges[r->m_id]->m_isAssignedFirst = assignmentIfFirst;
            }

            if(r->m_registerType != RegisterType::Count)
                result->m_ranges[r->m_id]->m_registerType = r->m_registerType;

            result->m_ranges[r->m_id]->m_range.second = i;
            result->m_ranges[r->m_id]->m_id = r->m_id;
            result->m_ranges[r->m_id]->m_typeInfo = r->m_type;
            result->m_ranges[r->m_id]->m_scale = r->m_scale;
            if(forceRegister != Operands::Register::Count)
                result->m_ranges[r->m_id]->m_forceTag = forceRegister;
            if(o.index() == 3 && std::get<Address>(o).m_index && r != std::get<Address>(o).m_index)
                r = std::get<Address>(o).m_index;
            else
                r = nullptr;
        }
    };


    for(size_t i = result->m_instructionRange.first; i <= result->m_instructionRange.second; i++) {
        if(i == 0) {
            for(auto& param : function.m_args)
                tryRegister(param, 0);
        }
        const std::unique_ptr<Instruction>& instr = function.m_instructions[i];
        
        switch(instr->m_type) {
            case Instruction::Point: {
                auto basic = static_cast<const BasicInstruction*>(instr.get());
                tryRegister(basic->m_o1, i, true);
                tryRegister(basic->m_o2, i);
                tryRegister(basic->m_o3, i);
                tryRegister(basic->m_o4, i);
                if(std::holds_alternative<std::shared_ptr<Register>>(basic->m_o2))
                    result->m_ranges[std::get<std::shared_ptr<Register>>(basic->m_o2)->m_id]->m_isPointedOrDereferenced = true;
                break;
            }
            case Instruction::PartialDereference: {
                auto basic = static_cast<const BasicInstruction*>(instr.get());
                // force this operand to be a spill register lol
                std::get<1>(basic->m_o1)->m_type = TypeInfo{LONG_T, 8, true};
                std::get<1>(basic->m_o1)->m_registerType = RegisterType::Signed;
                tryRegister(basic->m_o1, i, true, Operands::Register::R15);
                tryRegister(basic->m_o2, i);
                tryRegister(basic->m_o3, i);
                tryRegister(basic->m_o4, i);
                if(std::holds_alternative<Address>(basic->m_o2))
                    result->m_ranges[std::get<Address>(basic->m_o2).m_base->m_id]->m_isPointedOrDereferenced = true;
                break;
            }
            case Instruction::Dereference: {
                auto basic = static_cast<const BasicInstruction*>(instr.get());
                tryRegister(basic->m_o1, i, true);
                tryRegister(basic->m_o2, i);
                tryRegister(basic->m_o3, i);
                tryRegister(basic->m_o4, i);
                if(std::holds_alternative<Address>(basic->m_o2))
                    result->m_ranges[std::get<Address>(basic->m_o2).m_base->m_id]->m_isPointedOrDereferenced = true;
                break;
            }
            case Instruction::Call: {
                auto call = static_cast<const CallInstruction*>(instr.get());
                if(call->m_optReturn)
                    tryRegister(call->m_optReturn, i);

                for(auto& p : call->m_parameters) {
                    tryRegister(p, i);
                }
                break;
            }
            case Instruction::Upsize: {
                auto basic = static_cast<const BasicInstruction*>(instr.get());
                // std::get<1>(basic->m_o1)->m_type = TypeInfo{INT_T, 4, true};
                // std::get<1>(basic->m_o1)->m_registerType = RegisterType::Signed;
                tryRegister(basic->m_o1, i, true);
                tryRegister(basic->m_o2, i);
                tryRegister(basic->m_o3, i);
                tryRegister(basic->m_o4, i);
                break;
            }
            case Instruction::Label:
            case Instruction::Return:
                break;
            default: {
                auto basic = static_cast<const BasicInstruction*>(instr.get());
                tryRegister(basic->m_o1, i, true);
                tryRegister(basic->m_o2, i);
                tryRegister(basic->m_o3, i);
                tryRegister(basic->m_o4, i);
                break;
            }
        }
    }
}

}