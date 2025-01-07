#pragma once

#include "function_definition.hpp"

#include <functional>

class NativeFunctionNode : public FunctionDefinitionNode {
public:
    NativeFunctionNode(std::function<void(Stack&, Memory*, const std::vector<TypeInfo>&)> function) : m_function(std::move(function)) {}
public:
    std::function<void(Stack&, Memory*, const std::vector<TypeInfo>&)> m_function;
};