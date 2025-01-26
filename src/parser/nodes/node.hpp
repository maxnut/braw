#pragma once

#include <utility>
#include <cstdint>

namespace AST {

struct Node {
    enum Type {
        File,
        FunctionDefinition,
        VariableDeclaration,
        Scope,
        VariableAccess,
        FunctionCall,
        BinaryOperator,
        UnaryOperator,
        Literal,
        If,
        While,
        Struct,
        Return,
    };

    Node() = default;
    Node(Type t) : m_type(t) {} 
    virtual ~Node() = default;

    Type m_type;
    std::pair<uint32_t, uint32_t> m_rangeBegin;
    std::pair<uint32_t, uint32_t> m_rangeEnd;
};

}