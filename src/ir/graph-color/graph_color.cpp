#include "graph_color.hpp"
#include "ir/instruction.hpp"
#include "ir/instructions/binary.hpp"
#include "ir/instructions/call.hpp"
#include "ir/operator.hpp"
#include "ir/register.hpp"

#include <climits>
#include <cstdint>
#include <memory>
#include <stdexcept>

ColorResult GraphColor::build(const Function& function, std::vector<std::string> registers) {
    if(registers.size() < 3)
        throw std::runtime_error("Not enough registers");

    //reserve two registers for spill
    registers.erase(registers.end() - 2, registers.end());

    std::unordered_map<std::string, Range> ranges = fillRanges(function);
    ColorResult res;
    std::vector<GraphNode> stack;

    std::vector<GraphNode> graph;
    std::vector<GraphNode> spills;

    for(auto& range : ranges) {
        GraphNode node;
        node.m_id = range.first;
        node.m_connections = getOverlaps(node.m_id, ranges);
        graph.push_back(node);
    }

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

        for(const std::string& tag : registers) {
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

        graph.push_back(popped);
    }
    
    for(GraphNode& g : graph)
        res.m_registers.insert({g.m_id, g.m_tag});

    for(GraphNode& g : spills)
        res.m_spills.insert(g.m_id);

    return res;
}

std::unordered_map<std::string, Range> GraphColor::fillRanges(const Function& function) {
    std::unordered_map<std::string, Range> ranges;
    auto tryRegister = [&](Operator o, uint32_t i) {
        if(o.index() != 1)
            return;

        Register r = std::get<Register>(o);

        if(r.m_id == "%return") // the return register will always be rax/eax
            return;

        if(!ranges.contains(r.m_id))
            ranges[r.m_id].first = i;

        ranges[r.m_id].second = i;
    };

    for(uint32_t i = 0; i < function.m_instructions.size(); i++) {
        const std::unique_ptr<Instruction>& instr = function.m_instructions[i];
        
        switch(instr->m_type) {
            case Instruction::Call: {
                auto call = static_cast<const CallInstruction*>(instr.get());
                if(call->m_optReturn.m_id != "")
                    tryRegister(call->m_optReturn, i);
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

    return ranges;
}

std::vector<std::string> GraphColor::getOverlaps(const std::string& id, const std::unordered_map<std::string, Range>& ranges) {
    std::vector<std::string> ret;
    const Range& my = ranges.at(id);

    for(auto& r : ranges) {
        if(r.first == id)
            continue;

        Range cmp = r.second;

        if(my.first >= cmp.first && my.first <= cmp.second || my.second <= cmp.second && my.second >= cmp.first)
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