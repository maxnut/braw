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
        Cast,
        If,
        While,
        Struct,
        Bind,
        Return,
        Import
    };

    Node(Type t) : m_type(t) {} 

    Type m_type;
    std::pair<uint32_t, uint32_t> m_rangeBegin;
    std::pair<uint32_t, uint32_t> m_rangeEnd;
};

}