#pragma once

#include "parser/nodes/function_definition.hpp"
#include "parser/nodes/function_instruction.hpp"
#include "parser/nodes/evaluatable.hpp"
#include "stack.hpp"

class Interpreter {
public:
    void invokeFunction(std::shared_ptr<FunctionDefinitionNode> function, Stack& stack, Memory* returnValue, size_t functionPtr);
    void executeScope(std::unique_ptr<ScopeNode> scope, Stack& stack, FunctionContext& functionContext);
    void visitInstruction(std::unique_ptr<FunctionInstructionNode> instruction, Stack& stack, FunctionContext& functionContext);
    Memory evaluateInstruction(std::unique_ptr<EvaluatableNode> instruction, Stack& stack, FunctionContext& functionContext);

public:
    std::vector<Stack> m_stacks;
};