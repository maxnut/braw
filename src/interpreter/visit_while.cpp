#include "interpreter.hpp"
#include "parser/nodes/while.hpp"

void Interpreter::visitWhile(WhileNode* instruction, Stack& stack, FunctionContext& context) {
    void* head = stack.head();
    while(instruction->m_condition->evaluate(*this, stack, context).get<bool>()) {
        stack.pop((uintptr_t)stack.head() - (uintptr_t)head);
        executeScope(instruction->m_scope.get(), stack, context);
        if(context.m_return)
            break;
    }
    stack.pop((uintptr_t)stack.head() - (uintptr_t)head);
}