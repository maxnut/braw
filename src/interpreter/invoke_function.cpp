#include "interpreter/interpreter.hpp"
#include "parser/nodes/function_definition.hpp"
#include "parser/nodes/native_function.hpp"

#include <cstring>

void Interpreter::invokeFunction(FunctionDefinitionNode* function, Stack& stack, Memory* returnValue, size_t functionPtr) {
    FunctionContext context{};

    context.m_returnValue = stack.push(function->m_returnType.m_size);

    context.m_functionPtr = functionPtr;

    executeScope(function->m_scope.get(), stack, context);

    if(returnValue && context.m_returnValue.m_data)
        std::memcpy(returnValue->m_data, context.m_returnValue.m_data, context.m_returnValue.m_size);

    stack.pop(function->m_returnType.m_size);
}

void Interpreter::invokeNativeFunction(NativeFunctionNode* function, Stack& stack, Memory* returnValue, size_t functionPtr) {
    void* head = stack.head();
    Memory tempReturn = stack.push(function->m_returnType.m_size);
    function->m_function(stack, &tempReturn, function->m_parameters);
    if(returnValue)
        std::memcpy(returnValue->m_data, tempReturn.m_data, tempReturn.m_size);
    stack.setHead(head);
}