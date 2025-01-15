#include "interpreter.hpp"
#include "execution-tree/nodes/assignment.hpp"

#include <cstring>

void Interpreter::visitAssignment(AssignmentNode* instruction, Stack& stack, FunctionContext& context) {
    void* head = stack.head();
    Memory tmp = instruction->m_variable->evaluate(*this, stack, context);
    Memory m = instruction->m_value->evaluate(*this, stack, context);
    std::memcpy(tmp.m_data, m.m_data, m.m_size);
    stack.setHead(head);
}