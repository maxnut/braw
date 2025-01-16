#include "interpreter.hpp"
#include "execution-tree/nodes/return.hpp"

#include <cstring>

void Interpreter::visitReturn(ReturnNode* instruction, Stack& stack, FunctionContext& context) {
    context.m_return = true;
    if(!instruction->m_value)
        return;

    void* head = stack.head();
    Memory m = instruction->m_value->evaluate(*this, stack, context);
    std::memcpy(context.m_returnValue.m_data, m.m_data, context.m_returnValue.m_size);
    stack.setHead(head);
}