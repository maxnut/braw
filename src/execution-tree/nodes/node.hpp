#pragma once

#include "type_info.hpp"
#include "function_definition.hpp"

#include <unordered_map>
#include <vector>
#include <memory>
#include <string>

struct Node {
    enum Type {
        Address,
        Arrow,
        Assignment,
        BinaryOperator,
        Cast,
        Dereference,
        Dot,
        Evaluatable,
        File,
        FunctionCall,
        FunctionDefinition,
        If,
        Literal,
        NativeFunctionCall,
        NativeFunction,
        Return,
        Scope,
        VariableAccess,
        VariableDeclaration,
        While
    };

    Node(Type type) : m_type(type) {}

    Type m_type;
};