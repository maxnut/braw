#include "copy_propagator.hpp"
#include "ir/instruction.hpp"
#include "ssa/instruction.hpp"
#include "ssa/operand.hpp"
#include "ssa/operation.hpp"
#include <memory>

namespace SSA {

bool CopyPropagator::propagate(Function& function) {
    bool changed = false;
    size_t runs = 0;
    do {
        runs++;
        changed = false;
        for(auto block : function.m_blocks) {
            for(size_t i = block->m_instructionRange.first; i <= block->m_instructionRange.second; i++) {
                Instruction* ins = function.m_instructions.at(i).get();
                if(ins->m_type != Instruction::Assign || ((Assignment*)ins)->m_operation->m_type != Operation::Load || ((Assignment*)ins)->m_to->m_type != Operand::Register)
                    continue;
                Assignment* ass = (Assignment*)ins;

                if(function.m_retains.contains(std::static_pointer_cast<Register>(ass->m_to)->m_originalId)) continue;

                std::unordered_set<std::shared_ptr<Block>> visited;
                bool doErase = true;
                if(replace(block, function, i + 1, std::static_pointer_cast<Register>(ass->m_to), ass->m_operation->m_o1, visited, doErase)) {
                    function.m_instructions.erase(function.m_instructions.begin() + i);

                    for(auto block2 : function.m_blocks) {
                        if(block2->m_instructionRange.first > i) block2->m_instructionRange.first--;
                        if(block2->m_instructionRange.second > i) block2->m_instructionRange.second--;
                    }
                    
                    changed = true;
                    break;
                }
            }
            if(changed) break;
        }
    } while(changed);

    return runs > 1;
}

bool CopyPropagator::replace(std::shared_ptr<Block> block, Function& f, size_t start, std::shared_ptr<Register> repl, std::shared_ptr<Operand> with, std::unordered_set<std::shared_ptr<Block>>& visited, bool& doErase) {
    if(visited.contains(block))
        return false;
    visited.insert(block);

    bool replaced = false;

    auto tryReplace = [&](std::shared_ptr<Operand> op, bool& doErase) -> std::shared_ptr<Operand> {
        if(op->m_type != Operand::Register || std::static_pointer_cast<Register>(op)->m_id != repl->m_id)
            return op;

        if(std::static_pointer_cast<Register>(op)->m_memoryVersion != repl->m_memoryVersion) {
            doErase = false;
            return op;
        }

        replaced = true;
        return with;
    };
    
    for(size_t i = start; i <= block->m_instructionRange.second; i++) {
        Instruction* ins = f.m_instructions.at(i).get();
        switch(ins->m_type) {
            case Instruction::Assign: {
                Assignment* ass = (Assignment*)ins;
                Operation* oper = ass->m_operation.get();
                if(oper->m_type == Operation::Point) {
                    if(oper->m_o1->m_type == Operand::Register && std::static_pointer_cast<Register>(oper->m_o1)->m_id == repl->m_id)
                        doErase = false;
                    break;
                }

                oper->m_o1 = tryReplace(oper->m_o1, doErase);
                if(oper->m_o2)
                    oper->m_o2 = tryReplace(oper->m_o2, doErase);
                break;
            }
            case Instruction::Call: {
                Call* call = (Call*)ins;
                for(auto& param : call->m_parameters)
                    param = tryReplace(param, doErase); 
                break;
            }
            case Instruction::JumpFalse:
            case Instruction::JumpTrue: {
                Jump* jump = (Jump*)ins;
                jump->m_check = tryReplace(jump->m_check, doErase);
                break;
            }
            case Instruction::WriteMem: {
                WriteMem* write = (WriteMem*)ins;
                write->m_value = tryReplace(write->m_value, doErase);
                break;
            }
            case Instruction::Phi: {
                Phi* phi = (Phi*)ins;
                for(auto& op : phi->m_operands) {
                    if(op->m_type == Operand::Register && std::static_pointer_cast<Register>(op)->m_id == repl->m_id)
                        doErase = false;
                }
                break;
            }
            case Instruction::Return:
            case Instruction::Allocate:
            case Instruction::Jump:
            case Instruction::Label:
                break;
        }
    }

    for(auto con : block->m_dominated)
        replaced |= replace(con, f, con->m_instructionRange.first, repl, with, visited, doErase);

    return replaced && doErase;
}

}