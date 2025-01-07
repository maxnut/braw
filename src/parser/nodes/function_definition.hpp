#pragma once

#include "../../type_info.hpp"
#include "scope.hpp"

#include <vector>
#include <memory>

class FunctionDefinitionNode {
public:
    TypeInfo m_returnType;
    std::vector<TypeInfo> m_parameters;
    std::unique_ptr<ScopeNode> m_scope;
    std::string m_name;
};