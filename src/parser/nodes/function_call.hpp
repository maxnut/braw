#pragma once

#include "function_definition.hpp"
#include "evaluatable.hpp"

#include <memory>
#include <vector>

class FunctionCallNode : public EvaluatableNode {
public:
    std::shared_ptr<FunctionDefinitionNode> m_function;
    std::vector<std::unique_ptr<EvaluatableNode>> m_parameters;
};