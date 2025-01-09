#include "interpreter.hpp"
#include "parser/nodes/literal.hpp"

Memory Interpreter::visitLiteral(LiteralNode* instruction, Stack& stack, FunctionContext& context) {
    return instruction->m_value;
}