#pragma once

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

    Node(Type type) : m_nodeType(type) {}

    Type m_nodeType;
};