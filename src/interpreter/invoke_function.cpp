#include "interpreter/interpreter.hpp"
#include "parser/nodes/function_definition.hpp"
#include "parser/nodes/native_function.hpp"

#include <cstring>

void Interpreter::invokeFunction(FunctionDefinitionNode* function, Stack& stack, Memory* returnValue, size_t functionPtr) {
    FunctionContext context{};

    context.m_returnValue = stack.push(function->m_signature.m_returnType.m_size);
    context.m_functionPtr = functionPtr;

    executeScope(function->m_scope.get(), stack, context);

    if(returnValue && context.m_returnValue.m_data)
        std::memcpy(returnValue->m_data, context.m_returnValue.m_data, context.m_returnValue.m_size);

    stack.pop(function->m_signature.m_returnType.m_size);
}

void Interpreter::invokeNativeFunction(NativeFunctionNode* function, Stack& stack, Memory* returnValue, size_t functionPtr) {
    void* head = stack.head();
    function->m_function(stack, returnValue, function->m_signature.m_parameters);
    stack.setHead(head);
}