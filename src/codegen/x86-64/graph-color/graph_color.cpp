#include "graph_color.hpp"
#include "ir/address.hpp"
#include "ir/instruction.hpp"
#include "ir/instructions/basic.hpp"
#include "ir/instructions/call.hpp"
#include "ir/operand.hpp"
#include "ir/register.hpp"

#include <algorithm>
#include <climits>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <unordered_set>

namespace CodeGen::x86_64 {

ColorResult GraphColor::build(const Function& function, std::vector<Operands::Register::RegisterGroup> registers, std::vector<Operands::Register::RegisterGroup> precisionRegisters, int maxParamReg, int maxParamPReg) {
    if(registers.size() < 2 || precisionRegisters.size() < 2)
        throw std::runtime_error("Not enough registers");

    //reserve two registers for spill
    registers.erase(registers.end() - 1, registers.end());
    precisionRegisters.erase(precisionRegisters.end() - 1, precisionRegisters.end());

    ColorResult res;
    fillRanges(function, res);
    std::vector<GraphNode> stack;

    std::vector<GraphNode> graph;
    std::vector<GraphNode> spills;

    std::unordered_map<std::string, Operands::Register::RegisterGroup> paramAssignments;
    std::unordered_set<std::string> paramStack;

    int registerIndex = 0;
    int registerPrecisionIndex = 0;
    for (auto& param : function.m_args) {
        switch(res.m_ranges[param->m_id]->m_registerType) {
            case RegisterType::Single:
            case RegisterType::Double:
                if(registerPrecisionIndex >= precisionRegisters.size() || registerPrecisionIndex >= maxParamPReg) {
                    paramStack.insert(param->m_id);
                    break;
                }
                paramAssignments[param->m_id] = precisionRegisters[registerPrecisionIndex];
                registerPrecisionIndex++;
                break;
            default:
                if(registerIndex >= registers.size() || registerIndex >= maxParamReg) {
                    paramStack.insert(param->m_id);
                    break;
                }
                paramAssignments[param->m_id] = registers[registerIndex];
                registerIndex++;
                break;
        }
    }

    for(auto& range : res.m_rangeVector) {
        GraphNode node;
        node.m_registerType = range->m_registerType;
        node.m_id = range->m_id;
        if (paramAssignments.contains(node.m_id))
            node.m_tag = paramAssignments[node.m_id];

        if(paramStack.contains(node.m_id) || (node.m_registerType == RegisterType::Struct && !paramAssignments.contains(node.m_id))) {
            spills.push_back(node);
            res.m_ranges.erase(node.m_id);
            continue;
        }
        node.m_connections = getOverlaps(node.m_id, res.m_ranges);
        graph.push_back(node);
    }

    std::reverse(graph.begin(), graph.end());

    while(!graph.empty()) {
        bool removed = false;
        for(GraphNode& node : graph) {
            if(node.m_connections.size() < (node.m_registerType == RegisterType::Single || node.m_registerType == RegisterType::Double ? precisionRegisters.size() : registers.size())) {
                stack.push_back(node);
                removeFromGraph(node.m_id, graph);
                removed = true;
                break;
            }
        }

        if(!removed) {
            GraphNode relevant = getMostRelevantNode(graph);
            removeFromGraph(relevant.m_id, graph);
            spills.push_back(relevant);
        }
    }

    while(!stack.empty()) {
        GraphNode popped = stack.back();
        stack.pop_back();

        if(popped.m_tag != Operands::Register::Count) {
            graph.push_back(popped);
            continue;
        }

        auto tryTag([&](const std::vector<Operands::Register::RegisterGroup>& tags) {
            for(const auto& tag : tags) {
                bool found = false;
                for(auto& s : popped.m_connections) {
                    GraphNode node = findInGraph(s, graph);
                    if(node.m_tag == tag) {
                        found = true;
                        break;
                    }
                }

                if(!found) {
                    popped.m_tag = tag;
                    break;
                }
            }
        });

        switch(res.m_ranges[popped.m_id]->m_registerType) {
            case RegisterType::Single:
            case RegisterType::Double:
                tryTag(precisionRegisters);
                break;
            default:
                tryTag(registers);
                break;
        }

        graph.push_back(popped);
    }
    
    for(GraphNode& g : graph)
        res.m_registers.insert({g.m_id, g.m_tag});

    for(GraphNode& g : spills)
        res.m_spills.insert(g.m_id);

    return res;
}

void GraphColor::fillRanges(const Function& function, ColorResult& result) {
    auto tryRegister = [&](::Operand o, uint32_t i) {
        if(o.index() != 1 && o.index() != 3)
            return;

        auto r = o.index() == 3 ? std::get<Address>(o).m_base : std::get<std::shared_ptr<Register>>(o);

        if(r->m_id == "%return" || r->m_id == "%returnF") // the return register will always be rax/xmm0
            return;

        if(!result.m_ranges.contains(r->m_id)) {
            result.m_ranges[r->m_id] = std::make_shared<Range>();
            result.m_ranges[r->m_id]->m_range.first = i;
            result.m_rangeVector.push_back(result.m_ranges[r->m_id]);
        }

        if(r->m_registerType != RegisterType::Count)
            result.m_ranges[r->m_id]->m_registerType = r->m_registerType;

        result.m_ranges[r->m_id]->m_range.second = i;
        result.m_ranges[r->m_id]->m_id = r->m_id;
        result.m_ranges[r->m_id]->m_typeInfo = r->m_type;
    };

    for(auto& param : function.m_args)
        tryRegister(param, 0);

    for(uint32_t i = 0; i < function.m_instructions.size(); i++) {
        const std::unique_ptr<Instruction>& instr = function.m_instructions[i];
        
        switch(instr->m_type) {
            case Instruction::Call: {
                auto call = static_cast<const CallInstruction*>(instr.get());
                if(call->m_optReturn)
                    tryRegister(call->m_optReturn, i);

                for(auto& p : call->m_parameters)
                    tryRegister(p, i);
                break;
            }
            case Instruction::Label:
            case Instruction::Return:
                break;
            default: {
                auto basic = static_cast<const BasicInstruction*>(instr.get());
                tryRegister(basic->m_o1, i);
                tryRegister(basic->m_o2, i);
                tryRegister(basic->m_o3, i);
                tryRegister(basic->m_o4, i);
                break;
            }
        }
    }
}

std::vector<std::string> GraphColor::getOverlaps(const std::string& id, const std::unordered_map<std::string, std::shared_ptr<Range>>& ranges) {
    std::vector<std::string> ret;
    const auto my = ranges.at(id);

    for(auto& r : ranges) {
        if(r.first == id)
            continue;

        const auto cmp = r.second;

        if((cmp->m_registerType == RegisterType::Single || cmp->m_registerType == RegisterType::Double) && 
            (my->m_registerType != RegisterType::Single && my->m_registerType != RegisterType::Double))
            continue;

        if(my->m_range.first >= cmp->m_range.first && my->m_range.first <= cmp->m_range.second
            || my->m_range.second <= cmp->m_range.second && my->m_range.second >= cmp->m_range.first)
            ret.push_back(r.first);
    }

    return ret;
}

void GraphColor::removeFromGraph(const std::string& id, std::vector<GraphNode>& graph) {
    uint32_t idx = 0;

    for(idx = 0; idx < graph.size(); idx++) {
        GraphNode& node = graph[idx];
        std::vector<std::string>::iterator position = std::find(node.m_connections.begin(), node.m_connections.end(), id);
        if (position != node.m_connections.end())
            node.m_connections.erase(position);
    }

    graph.erase(std::remove_if(graph.begin(), graph.end(), [&](GraphNode& node) { return node.m_id == id; }), graph.end());
}

GraphNode GraphColor::getMostRelevantNode(std::vector<GraphNode>& graph) {
    int max = INT_MIN;
    GraphNode res;

    for(GraphNode& node : graph) {
        if((int)node.m_connections.size() > max) {
            max = node.m_connections.size();
            res = node;
        }
    }

    return res;
}

GraphNode GraphColor::findInGraph(const std::string& id, std::vector<GraphNode>& graph) {
    for(GraphNode& node : graph) {
        if(node.m_id == id)
            return node;
    }

    return {};
}

}