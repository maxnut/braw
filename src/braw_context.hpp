#pragma once

#include "parser/nodes/function_definition.hpp"
#include "type_info.hpp"

#include <memory>
#include <unordered_map>
#include <vector>
#include <optional>

struct ScopeInfo {
    size_t getSize() const {return m_type.m_size * (m_arraySize == 0 ? 1 : m_arraySize);}

    TypeInfo m_type;
    size_t m_stackOffset;
    size_t m_arraySize;
};

struct FunctionSignature {
    TypeInfo m_returnType;
    std::vector<TypeInfo> m_parameters;
    std::vector<std::string> m_parameterNames;
    std::string m_name;
};

enum Assembler {
    NASM,
    GAS
};

struct BrawContext {
    BrawContext();

    std::optional<ScopeInfo> getScopeInfo(const std::string& name) const;
    std::optional<TypeInfo> getTypeInfo(const std::string& name) const;
    std::shared_ptr<FunctionSignature> getFunction(const std::string& name, const std::vector<TypeInfo>& parameters) const;
    bool isDefinedInScope(const std::string& name) const;
    bool functionExists(std::shared_ptr<FunctionSignature> func) const;

    std::vector<std::unordered_map<std::string, ScopeInfo>> m_scopes;
    std::unordered_map<std::string, TypeInfo> m_typeTable;
    std::unordered_map<std::string, std::vector<std::shared_ptr<FunctionSignature>>> m_functionTable;
    std::shared_ptr<FunctionSignature> m_currentFunction = nullptr;
    Assembler m_assembler = NASM;
    
    size_t m_stackSize = 0;
};