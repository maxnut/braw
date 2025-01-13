#include "interpreter.hpp"
#include "parser/nodes/cast.hpp"

Memory Interpreter::visitCast(CastNode* instruction, Stack& stack, FunctionContext& context) {
    void* head = stack.head();
    Memory m = instruction->m_base->evaluate(*this, stack, context);
    stack.setHead(head);
    return instruction->m_function(m, stack);
}