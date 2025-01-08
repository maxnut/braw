#include "interpreter.hpp"
#include "parser/nodes/address.hpp"

Memory Interpreter::visitAddress(AddressNode* instruction, Stack& stack, FunctionContext& context) {
    void* head = stack.head();
    Memory tmp = instruction->m_base->evaluate(*this, stack, context);
    stack.setHead(head);
    Memory m = stack.push(instruction->m_size);
    m.from(tmp.m_data);
    return m;
}