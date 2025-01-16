#include "interpreter/interpreter.hpp"
#include "execution-tree/nodes/scope.hpp"

void Interpreter::executeScope(ScopeNode* scope, Stack& stack, FunctionContext& functionContext) {
    void* head = stack.head();
    for(auto& node : scope->m_instructions) {
        node->visit(*this, stack, functionContext);

        if(functionContext.m_return)
            break;
    }
    stack.setHead(head);
}