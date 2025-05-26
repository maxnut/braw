#include "braw_context.hpp"
#include "rules.hpp"

#include <memory>

BrawContext::BrawContext() {
    m_typeTable = {
        {VOID_T, TypeInfo{VOID_T, 0, true}},

        {INT_T, TypeInfo{INT_T, 4, true,
            {
                {"+", {INT_T}},
                {"-", {INT_T}},
                {"*", {INT_T} },
                {"/", {INT_T}},
                {"%", {INT_T}},
                {"&", {INT_T}},
                {"|", {INT_T}},
                {"^", {INT_T}},
                {"==", {BOOL_T}},
                {"!=", {BOOL_T}},
                {">", {BOOL_T}},
                {">=", {BOOL_T}},
                {"<", {BOOL_T}},
                {"<=", {BOOL_T}}
            },
            {LONG_T,BOOL_T,CHAR_T, UINT_T, ULONG_T, UCHAR_T}
        }},

        {LONG_T, TypeInfo{LONG_T, 8, true,
            {
                {"+", {LONG_T}},
                {"-", {LONG_T}},
                {"*", {LONG_T} },
                {"/", {LONG_T}},
                {"%", {LONG_T}},
                {"&", {LONG_T}},
                {"|", {LONG_T}},
                {"^", {LONG_T}},
                {"==", {BOOL_T}},
                {"!=", {BOOL_T}},
                {">", {BOOL_T}},
                {">=", {BOOL_T}},
                {"<", {BOOL_T}},
                {"<=", {BOOL_T}}
            },
            {INT_T,BOOL_T,CHAR_T, UINT_T, ULONG_T, UCHAR_T}
        }},

        {CHAR_T, TypeInfo{CHAR_T, 1, true,
            {
                {"+", {CHAR_T}},
                {"-", {CHAR_T}},
                {"*", {CHAR_T} },
                {"/", {CHAR_T}},
                {"%", {CHAR_T}},
                {"&", {CHAR_T}},
                {"|", {CHAR_T}},
                {"^", {CHAR_T}},
                {"==", {BOOL_T}},
                {"!=", {BOOL_T}},
                {">", {BOOL_T}},
                {">=", {BOOL_T}},
                {"<", {BOOL_T}},
                {"<=", {BOOL_T}}
            },
            {INT_T,BOOL_T,CHAR_T, UINT_T, ULONG_T, UCHAR_T}
        }},

        {UINT_T, TypeInfo{UINT_T, 4, true,
            {
                {"+", {UINT_T}},
                {"-", {UINT_T}},
                {"*", {UINT_T} },
                {"/", {UINT_T}},
                {"%", {UINT_T}},
                {"&", {UINT_T}},
                {"|", {UINT_T}},
                {"^", {UINT_T}},
                {"==", {BOOL_T}},
                {"!=", {BOOL_T}},
                {">", {BOOL_T}},
                {">=", {BOOL_T}},
                {"<", {BOOL_T}},
                {"<=", {BOOL_T}}
            },
            {ULONG_T,BOOL_T,UCHAR_T, INT_T, LONG_T, CHAR_T}
        }},

        {ULONG_T, TypeInfo{ULONG_T, 8, true,
            {
                {"+", {ULONG_T}},
                {"-", {ULONG_T}},
                {"*", {ULONG_T} },
                {"/", {ULONG_T}},
                {"%", {ULONG_T}},
                {"&", {ULONG_T}},
                {"|", {ULONG_T}},
                {"^", {ULONG_T}},
                {"==", {BOOL_T}},
                {"!=", {BOOL_T}},
                {">", {BOOL_T}},
                {">=", {BOOL_T}},
                {"<", {BOOL_T}},
                {"<=", {BOOL_T}}
            },
            {UINT_T,BOOL_T,UCHAR_T, INT_T, LONG_T, CHAR_T}
        }},

        {UCHAR_T, TypeInfo{UCHAR_T, 1, true,
            {
                {"+", {UCHAR_T}},
                {"-", {UCHAR_T}},
                {"*", {UCHAR_T} },
                {"/", {UCHAR_T}},
                {"%", {UCHAR_T}},
                {"&", {UCHAR_T}},
                {"|", {UCHAR_T}},
                {"^", {UCHAR_T}},
                {"==", {BOOL_T}},
                {"!=", {BOOL_T}},
                {">", {BOOL_T}},
                {">=", {BOOL_T}},
                {"<", {BOOL_T}},
                {"<=", {BOOL_T}}
            },
            {UINT_T,BOOL_T,UCHAR_T, INT_T, LONG_T, CHAR_T}
        }},

        {FLOAT_T, TypeInfo{FLOAT_T, 4, true,
            {
                {"+", {FLOAT_T}},
                {"-", {FLOAT_T}},
                {"*", {FLOAT_T}},
                {"/", {FLOAT_T}},
                {"==", {BOOL_T}},
                {"!=", {BOOL_T}},
                {">", {BOOL_T}},
                {">=", {BOOL_T}},
                {"<", {BOOL_T}},
                {"<=", {BOOL_T}}
            },
            {INT_T,LONG_T,DOUBLE_T,BOOL_T}
        }},

        {DOUBLE_T, TypeInfo{DOUBLE_T, 8, true,
            {
                {"+", {DOUBLE_T}},
                {"-", {DOUBLE_T}},
                {"*", {DOUBLE_T}},
                {"/", {DOUBLE_T}},
                {"==", {BOOL_T}},
                {"!=", {BOOL_T}},
                {">", {BOOL_T}},
                {">=", {BOOL_T}},
                {"<", {BOOL_T}},
                {"<=", {BOOL_T}}
            },
            {INT_T,LONG_T,FLOAT_T,BOOL_T}
        }},

        {BOOL_T, TypeInfo{BOOL_T, 1, true,
            {
                {"&", {BOOL_T}},
                {"|", {BOOL_T}},
                {"^", {BOOL_T}},
                {"==", {BOOL_T}},
                {"!=", {BOOL_T}},
                {"&&", {BOOL_T}},
                {"||", {BOOL_T}}
            }
        }}
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
        return TypeInfo(name, 8, true);

    if(m_typeTable.contains(name))
        return m_typeTable.at(name);

    return std::nullopt;
}