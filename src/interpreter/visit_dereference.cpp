#include "interpreter.hpp"
#include "parser/nodes/dereference.hpp"

Memory Interpreter::visitDereference(DereferenceNode* instruction, Stack& stack, FunctionContext& context) {
    void* head = stack.head();
    Memory m = instruction->m_base->evaluate(*this, stack, context);
    void* ptr = m.get<void*>();
    stack.setHead(head);

    m.m_size = instruction->m_size;
    m.m_data = ptr;
    return m;
}