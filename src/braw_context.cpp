#include "braw_context.hpp"
#include "rules.hpp"

#include <memory>

BrawContext::BrawContext() {
    m_typeTable = {
        {"void", TypeInfo{"void", 0}},

        {"int", TypeInfo{"int", 8, //TODO: this should be 4, but until i have proper register size handling force 8 
            {
                {"+", {"int"}},
                {"-", {"int"}},
                {"*", {"int"} },
                {"/", {"int"}},
                {"==", {"bool"}},
                {"!=", {"bool"}},
                {">", {"bool"}},
                {">=", {"bool"}},
                {"<", {"bool"}},
                {"<=", {"bool"}}
            },
            {"float", "double", "long", "bool"}
        }},

        {"long", TypeInfo{"long", 8, 
            {
                {"+", {"long"}},
                {"-", {"long"}},
                {"*", {"long"} },
                {"/", {"long"}},
                {"==", {"bool"}},
                {"!=", {"bool"}},
                {">", {"bool"}},
                {">=", {"bool"}},
                {"<", {"bool"}},
                {"<=", {"bool"}}
            },
            {"int", "float", "double", "bool"}
        }},

        {"float", TypeInfo{"float", 4, 
            {
                {"+", {"float"}},
                {"-", {"float"}},
                {"*", {"float"}},
                {"/", {"float"}},
                {"==", {"bool"}},
                {"!=", {"bool"}},
                {">", {"bool"}},
                {">=", {"bool"}},
                {"<", {"bool"}},
                {"<=", {"bool"}}
            },
            {"int", "long", "double", "bool"}
        }},

        {"double", TypeInfo{"double", 8, 
            {
                {"+", {"double"}},
                {"-", {"double"}},
                {"*", {"double"}},
                {"/", {"double"}},
                {"==", {"bool"}},
                {"!=", {"bool"}},
                {">", {"bool"}},
                {">=", {"bool"}},
                {"<", {"bool"}},
                {"<=", {"bool"}}
            },
            {"int", "long", "float", "bool"}
        }},

        {"bool", TypeInfo{"bool", 1}},

        {"char", TypeInfo{"char", 1}}
    };
}

bool BrawContext::functionExists(std::shared_ptr<FunctionSignature> function) const {
    if(!m_functionTable.contains(function->m_name))
        return false;

    bool found = true;

    for(auto func : m_functionTable.at(function->m_name)) {
        if(func->m_parameters.size() != function->m_parameters.size()) {
            found = false;
            continue;
        }

        for(int i = 0; i < func->m_parameters.size(); i++) {
            if(func->m_parameters[i] != function->m_parameters[i]) {
                found = false;
                continue;
            }
        }
        break;
    }

    return found;
}

std::optional<ScopeInfo> BrawContext::getScopeInfo(const std::string& name) const {
    for(auto it = m_scopes.rbegin(); it != m_scopes.rend(); it++) {
        auto it2 = it->find(name);
        if(it2 != it->end()) {
            return it2->second;
        }
    }
    return std::nullopt;
}

bool BrawContext::isDefinedInScope(const std::string& name) const {
    return getScopeInfo(name).has_value();
}

std::shared_ptr<FunctionSignature> BrawContext::getFunction(const std::string& name, const std::vector<TypeInfo>& parameters) const {
    auto check = [&](std::shared_ptr<FunctionSignature> func) -> bool {
        if(func->m_parameters.size() != parameters.size())
            return false;

        for(int i = 0; i < func->m_parameters.size(); i++) {
            if(!(Rules::isPtr(func->m_parameters[i].m_name) && Rules::isPtr(parameters[i].m_name)) && func->m_parameters[i] != parameters[i])
                return false;
        }
        
        return true;
    };

    if(m_functionTable.contains(name)) {
        for(std::shared_ptr<FunctionSignature> func : m_functionTable.at(name)) {
            if(check(func))
                return func;
        }
    }

    return nullptr;
}

std::optional<TypeInfo> BrawContext::getTypeInfo(const std::string& name) const {
    if(Rules::isPtr(name))
        return TypeInfo(name, 8);

    if(m_typeTable.contains(name))
        return m_typeTable.at(name);

    return std::nullopt;
}