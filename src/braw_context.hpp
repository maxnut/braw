#pragma once

#include "execution-tree/nodes/evaluatable.hpp"
#include "type_info.hpp"
#include "execution-tree/nodes/function_definition.hpp"

#include <memory>
#include <unordered_map>
#include <optional>

struct ScopeInfo {
    size_t getSize() const {return m_type.m_size * (m_arraySize == 0 ? 1 : m_arraySize);}

    TypeInfo m_type;
    size_t m_stackOffset;
    size_t m_arraySize;
};

struct BrawContext {
    BrawContext();

    std::optional<ScopeInfo> getScopeInfo(const std::string& name) const;
    std::optional<TypeInfo> getTypeInfo(const std::string& name) const;
    std::shared_ptr<FunctionDefinitionNode> getFunction(const std::string& name, const std::vector<TypeInfo>& parameters) const;
    std::shared_ptr<FunctionDefinitionNode> getFunction(const std::string& name, const std::vector<std::unique_ptr<EvaluatableNode>>& parameters) const;
    bool isDefinedInScope(const std::string& name) const;
    bool functionExists(std::shared_ptr<FunctionDefinitionNode> func) const;

    std::vector<std::unordered_map<std::string, ScopeInfo>> m_scopes;
    std::unordered_map<std::string, TypeInfo> m_typeTable;
    std::unordered_map<std::string, std::vector<std::shared_ptr<FunctionDefinitionNode>>> m_functionTable;
    
    size_t m_stackSize = 0;
};