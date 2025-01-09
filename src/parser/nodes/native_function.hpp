#pragma once

#include "function_definition.hpp"

#include <functional>

class NativeFunctionNode : public FunctionDefinitionNode {
public:
    NativeFunctionNode() = default;
    NativeFunctionNode(const std::string& name, std::vector<TypeInfo> parameters, TypeInfo returnType, std::function<void(Stack&, Memory*, const std::vector<TypeInfo>&)> function) : m_function(std::move(function)) {
        m_signature.m_name = name;
        m_signature.m_parameters = std::move(parameters);
        m_signature.m_returnType = std::move(returnType);
    }

    bool isNative() override {return true;}

public:
    std::function<void(Stack&, Memory*, const std::vector<TypeInfo>&)> m_function;
};