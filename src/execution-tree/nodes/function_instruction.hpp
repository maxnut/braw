#pragma once

#include "interpreter/interpreter.hpp"
#include "node.hpp"

class FunctionInstructionNode : public Node {
public:
    FunctionInstructionNode(Type type) : Node(type) {}

    virtual ~FunctionInstructionNode() = default;
    
    virtual void visit(Interpreter& interpreter, Stack& stack, FunctionContext& functionContext) = 0;
};