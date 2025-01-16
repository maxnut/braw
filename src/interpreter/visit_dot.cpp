#include "interpreter.hpp"
#include "execution-tree/nodes/dot.hpp"

Memory Interpreter::visitDot(DotNode* instruction, Stack& stack, FunctionContext& context) {
    Memory m = instruction->m_base->evaluate(*this, stack, context);
    return Memory{(void*)((uintptr_t)m.m_data + instruction->m_offset), instruction->m_size};
}