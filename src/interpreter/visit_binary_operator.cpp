#include "interpreter.hpp"
#include "execution-tree/nodes/binary_operator.hpp"

Memory Interpreter::visitBinaryOperator(BinaryOperatorNode* instruction, Stack& stack, FunctionContext& context) {
    Memory eval = stack.push(instruction->m_left->m_size);

    void* head = stack.head();

    Memory left = instruction->m_left->evaluate(*this, stack, context);
    Memory right = instruction->m_right->evaluate(*this, stack, context);

    instruction->m_function(eval, left, right);

    stack.setHead(head);

    return eval;
}