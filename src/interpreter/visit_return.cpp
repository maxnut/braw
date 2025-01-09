#include "interpreter.hpp"
#include "parser/nodes/return.hpp"

#include <cstring>

void Interpreter::visitReturn(ReturnNode* instruction, Stack& stack, FunctionContext& context) {
    if(!instruction->m_value)
        return;

    void* head = stack.head();
    Memory m = instruction->m_value->evaluate(*this, stack, context);
    std::memcpy(context.m_returnValue.m_data, m.m_data, context.m_returnValue.m_size);
    stack.setHead(head);
    context.m_return = true;
}