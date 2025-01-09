#include "interpreter.hpp"
#include "parser/nodes/arrow.hpp"

Memory Interpreter::visitArrow(ArrowNode* instruction, Stack& stack, FunctionContext& context) {
    void* head = stack.head();
    Memory m = instruction->m_base->evaluate(*this, stack, context);
    void* ptr = m.get<void*>();
    stack.setHead(head);
    return Memory{(void*)((uintptr_t)ptr + instruction->m_offset), instruction->m_size};
}