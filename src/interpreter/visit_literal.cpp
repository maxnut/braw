#include "interpreter.hpp"
#include "execution-tree/nodes/literal.hpp"

Memory Interpreter::visitLiteral(LiteralNode* instruction, Stack& stack, FunctionContext& context) {
    return instruction->m_value;
}