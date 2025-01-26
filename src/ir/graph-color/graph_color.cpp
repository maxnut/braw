#include "graph_color.hpp"
#include "ir/instruction.hpp"
#include "ir/instructions/binary.hpp"
#include "ir/instructions/call.hpp"
#include "ir/operand.hpp"
#include "ir/register.hpp"

#include <algorithm>
#include <climits>
#include <cstdint>
#include <memory>
#include <stdexcept>

ColorResult GraphColor::build(const Function& function, std::vector<std::string> registers, std::vector<std::string> precisionRegisters) {
    if(registers.size() < 3 || precisionRegisters.size() < 3)
        throw std::runtime_error("Not enough registers");

    //reserve two registers for spill
    registers.erase(registers.end() - 2, registers.end());
    precisionRegisters.erase(precisionRegisters.end() - 2, precisionRegisters.end());

    ColorResult res;
    fillRanges(function, res);
    std::vector<GraphNode> stack;

    std::vector<GraphNode> graph;
    std::vector<GraphNode> spills;

    //TODO: ignore stack parameters
    std::unordered_map<std::string, std::string> paramAssignments;

    int registerIndex = 0;
    int registerPrecisionIndex = 0;
    for (auto& param : function.m_args) {
        switch(res.m_ranges[param->m_id].m_registerType) {
            case RegisterType::Single:
            case RegisterType::Double:
                paramAssignments[param->m_id] = precisionRegisters[registerPrecisionIndex];
                registerPrecisionIndex++;
                break;
            default:
                paramAssignments[param->m_id] = registers[registerIndex];
                registerIndex++;
                break;
        }
    }

    for(auto& range : res.m_rangeVector) {
        GraphNode node;
        node.m_id = range->m_id;
        if (paramAssignments.contains(node.m_id))
            node.m_tag = paramAssignments[node.m_id];

        node.m_connections = getOverlaps(node.m_id, res.m_ranges);
        graph.push_back(node);
    }

    std::reverse(graph.begin(), graph.end());

    while(!graph.empty()) {
        bool removed = false;
        for(GraphNode& node : graph) {
            if(node.m_connections.size() < registers.size()) {
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

        if(popped.m_tag != "") {
            graph.push_back(popped);
            continue;
        }

        auto tryTag([&](const std::vector<std::string>& tags) {
            for(const std::string& tag : tags) {
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

        switch(res.m_ranges[popped.m_id].m_registerType) {
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
    auto tryRegister = [&](Operand o, uint32_t i) {
        if(o.index() != 1)
            return;

        auto r = std::get<std::shared_ptr<Register>>(o);

        if(r->m_id == "%return") // the return register will always be rax/eax
            return;

        if(!result.m_ranges.contains(r->m_id)) {
            result.m_ranges[r->m_id].m_range.first = i;
            result.m_rangeVector.push_back(&result.m_ranges[r->m_id]);
        }

        if(r->m_registerType != RegisterType::Count)
            result.m_ranges[r->m_id].m_registerType = r->m_registerType;

        result.m_ranges[r->m_id].m_range.second = i;
        result.m_ranges[r->m_id].m_id = r->m_id;
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
            case Instruction::Add:
            case Instruction::Move:
            case Instruction::Subtract:
            case Instruction::Multiply:
            case Instruction::Point:
            case Instruction::CompareEquals:
            case Instruction::CompareGreaterEquals:
            case Instruction::CompareLessEquals:
            case Instruction::CompareNotEquals:
            case Instruction::JumpFalse: {
                auto binary = static_cast<const BinaryInstruction*>(instr.get());
                tryRegister(binary->m_left, i);
                tryRegister(binary->m_right, i);
                break;
            }
            default:
                break;
        }
    }
}

std::vector<std::string> GraphColor::getOverlaps(const std::string& id, const std::unordered_map<std::string, Range>& ranges) {
    std::vector<std::string> ret;
    const Range& my = ranges.at(id);

    for(auto& r : ranges) {
        if(r.first == id)
            continue;

        Range cmp = r.second;

        if(my.m_range.first >= cmp.m_range.first && my.m_range.first <= cmp.m_range.second
            || my.m_range.second <= cmp.m_range.second && my.m_range.second >= cmp.m_range.first)
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
        if(node.m_connections.size() > max) {
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