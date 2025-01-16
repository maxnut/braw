#pragma once

#include "../../type_info.hpp"
#include "scope.hpp"

#include <vector>
#include <memory>

struct FunctionSignature {
    TypeInfo m_returnType;
    std::vector<TypeInfo> m_parameters;
    std::vector<std::string> m_parameterNames;
    std::string m_name;
};

class FunctionDefinitionNode {
public:
    virtual bool isNative() {return false;}
public:
    FunctionSignature m_signature;
    std::unique_ptr<ScopeNode> m_scope;
};