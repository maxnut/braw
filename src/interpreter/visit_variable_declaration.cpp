#include "interpreter.hpp"
#include "execution-tree/nodes/variable_declaration.hpp"

#include <cstring>

void Interpreter::visitVariableDeclaration(VariableDeclarationNode* instruction, Stack& stack, FunctionContext& context) {
    Memory var = stack.push(instruction->size);

    if(instruction->m_assignmentValue) {
        void* head = stack.head();
        Memory m = instruction->m_assignmentValue->evaluate(*this, stack, context);
        std::memcpy(var.m_data, m.m_data, m.m_size);
        stack.setHead(head);
    }
}