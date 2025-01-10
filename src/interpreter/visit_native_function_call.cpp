#include "interpreter.hpp"
#include "parser/nodes/function_call.hpp"
#include "parser/nodes/native_function_call.hpp"

#include <cstring>

Memory Interpreter::visitNativeFunctionCall(NativeFunctionCallNode* instruction, Stack& stack, FunctionContext& context) {
    Memory ret = stack.push(instruction->m_size);
    void* head = stack.head();
    size_t funcPtr = (uintptr_t)stack.head() - (uintptr_t)stack.data();

    for(auto& param : instruction->m_parameters) {
        void* head = stack.head();
        Memory m = param->evaluate(*this, stack, context);
        if(head == stack.head()) { // did not push in stack
            Memory p = stack.push(m.m_size);
            std::memcpy(p.m_data, m.m_data, m.m_size);
        }
    }

    auto func = (NativeFunctionNode*)instruction->m_function.get();
    invokeNativeFunction(func, stack, &ret, funcPtr);
    
    stack.setHead(head);
    return ret;
}