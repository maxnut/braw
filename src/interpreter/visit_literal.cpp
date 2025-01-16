#include "interpreter.hpp"
#include "execution-tree/nodes/literal.hpp"
#include "memory.hpp"

Memory Interpreter::visitLiteral(LiteralNode* instruction, Stack& stack, FunctionContext& context) {
    switch(instruction->m_value.index()) {
        case 5: {
            instruction->m_strPtr = std::get<std::string>(instruction->m_value).data();
            Memory m{&instruction->m_strPtr, instruction->m_size};
            return m;
        }
        default:
            break;
    }

    return Memory{
                std::visit([](auto& value) -> void* { return &value; }, instruction->m_value),
                instruction->m_size
            };
}