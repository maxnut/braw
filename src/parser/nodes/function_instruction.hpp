#pragma once

#include "interpreter/interpreter.hpp"

class FunctionInstructionNode {
public:
    virtual void visit(Interpreter& interpreter, Stack& stack, FunctionContext& functionContext) = 0;
};