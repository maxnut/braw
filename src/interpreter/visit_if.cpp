#include "interpreter.hpp"
#include "execution-tree/nodes/if.hpp"

void Interpreter::visitIf(IfNode* instruction, Stack& stack, FunctionContext& context) {
    void* head = stack.head();
    Memory eval = instruction->m_condition->evaluate(*this, stack, context);
    if(eval.get<bool>()) {
        stack.setHead(head);
        executeScope(instruction->m_scope.get(), stack, context);
        return;
    }
        
    stack.setHead(head);
}