#include "codegen/x86-64/register-allocator/register_allocator.hpp"
#include "codegen/x86-64/register.hpp"
#include "ir/register.hpp"

#include <algorithm>
#include <climits>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <unordered_set>

namespace CodeGen::x86_64 {

RegisterAllocatorResult RegisterAllocator::build(const Function& function, std::vector<Operands::Register::RegisterGroup> registers, std::vector<Operands::Register::RegisterGroup> precisionRegisters, int maxParamReg, int maxParamPReg) {
    if(registers.size() < 2 || precisionRegisters.size() < 2)
        throw std::runtime_error("Not enough registers");

    //reserve two registers for spill
    registers.erase(registers.end() - 1, registers.end());
    precisionRegisters.erase(precisionRegisters.end() - 1, precisionRegisters.end());

    RegisterAllocatorResult res;
    res.m_propagated = Propagator::buildGraph(function);
    for(auto& block : res.m_propagated.blocks) {
        std::vector<GraphNode> stack;

        std::vector<GraphNode> graph;
        std::vector<GraphNode> spills;

        std::unordered_map<std::string, Operands::Register::RegisterGroup> paramAssignments;
        std::unordered_set<std::string> paramStack;

        int registerIndex = 0;
        int registerPrecisionIndex = 0;
        for (auto& param : function.m_args) {
            if(!block->m_ranges.contains(param->m_id))
                continue;
            switch(block->m_ranges[param->m_id]->m_registerType) {
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

        for(auto& range : block->m_rangeVector) {
            GraphNode node;
            node.m_registerType = range->m_registerType;
            node.m_id = range->m_id;
            if (paramAssignments.contains(node.m_id))
                node.m_tag = paramAssignments[node.m_id];

            if(range->m_forceTag != Operands::Register::Count)
                node.m_tag = range->m_forceTag;
            else if(res.m_registers.contains(node.m_id))
                node.m_tag = res.m_registers.at(node.m_id);
            else if((!paramAssignments.contains(node.m_id) && range->m_isPointedOrDereferenced) || paramStack.contains(node.m_id) || ((node.m_registerType == RegisterType::Struct || node.m_registerType == RegisterType::Pointer || range->m_scale > 1) && !paramAssignments.contains(node.m_id))) {
                spills.push_back(node);
                block->m_ranges.erase(node.m_id);
                continue;
            }
            node.m_connections = getOverlaps(node.m_id, block->m_ranges);
            graph.push_back(node);
        }

        std::sort(graph.begin(), graph.end(), [](const GraphNode& a, const GraphNode& b) {
            if(a.m_tag == Operands::Register::Count || b.m_tag == Operands::Register::Count) {
                if(a.m_tag != Operands::Register::Count)
                    return false;
                if(b.m_tag != Operands::Register::Count)
                    return true;
            }
            return a.m_connections.size() > b.m_connections.size();
        });

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

            switch(block->m_ranges[popped.m_id]->m_registerType) {
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
    }

    return res;
}

std::vector<std::string> RegisterAllocator::getOverlaps(const std::string& id, const std::unordered_map<std::string, std::shared_ptr<Range>>& ranges) {
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

void RegisterAllocator::removeFromGraph(const std::string& id, std::vector<GraphNode>& graph) {
    uint32_t idx = 0;

    for(idx = 0; idx < graph.size(); idx++) {
        GraphNode& node = graph[idx];
        std::vector<std::string>::iterator position = std::find(node.m_connections.begin(), node.m_connections.end(), id);
        if (position != node.m_connections.end())
            node.m_connections.erase(position);
    }

    graph.erase(std::remove_if(graph.begin(), graph.end(), [&](GraphNode& node) { return node.m_id == id; }), graph.end());
}

GraphNode RegisterAllocator::getMostRelevantNode(std::vector<GraphNode>& graph) {
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

GraphNode RegisterAllocator::findInGraph(const std::string& id, std::vector<GraphNode>& graph) {
    for(GraphNode& node : graph) {
        if(node.m_id == id)
            return node;
    }

    return {};
}

}