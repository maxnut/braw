#pragma once

#include "stack.hpp"
#include "type_info.hpp"
#include "braw_context.hpp"
#include "parser/nodes/function_definition.hpp"

#include <string>
#include <optional>

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

    inline TypeInfo makePointer(const TypeInfo& base) {
        return TypeInfo{base.m_name + "*", 8};
    }

    inline std::optional<TypeInfo> getRawType(const TypeInfo& pointer, const BrawContext& ctx) {
        std::string raw = pointer.m_name;
        if(raw.find("*") != std::string::npos)
            raw = raw.substr(0, raw.size() - 1);

        if(raw.find("*") != std::string::npos)
            return TypeInfo{raw, 8};


        return ctx.getTypeInfo(raw);
    }
}