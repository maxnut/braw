#include "interpreter.hpp"
#include "execution-tree/nodes/literal.hpp"

Memory Interpreter::visitLiteral(LiteralNode* instruction, Stack& stack, FunctionContext& context) {
    void* data = nullptr;
    std::visit([&data](auto&& value) { data = &value; }, instruction->m_value);
    return Memory{data, instruction->m_size};
}