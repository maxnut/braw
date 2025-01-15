#pragma once

#include "stack.hpp"
#include "parser/nodes/function_definition.hpp"

#include <string>

namespace Utils {
    template <typename T, size_t offset>
    inline T fromStack(Stack& stack) {
        return *(T*)(stack.ptrHead(offset));
    }

    inline std::string functionSignatureString(AST::FunctionSignature signature) {
        std::string funcString = signature.m_name.m_name + "(";
        for(int i = 0; i < signature.m_parameters.size(); i++) {
            auto& parameter = signature.m_parameters[i];
            funcString += parameter.m_type.m_name;
            if(parameter.m_name.m_name.size() > 0)
                funcString += " " + parameter.m_name.m_name;
            
            if(i < signature.m_parameters.size() - 1)
                funcString += ", ";
        }
        funcString += ")";
        return funcString;
    }
}