#include "interpreter.hpp"
#include "execution-tree/nodes/variable_access.hpp"

Memory Interpreter::visitVariableAccess(VariableAccessNode* instruction, Stack& stack, FunctionContext& context) {
    return Memory{stack.ptr(context.m_functionPtr + instruction->m_stackIndex), instruction->m_size};
}