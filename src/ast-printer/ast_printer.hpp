#pragma once

#include "parser/nodes/node.hpp"

#include <string>
#include <memory>

namespace AST {
struct FileNode;
struct FunctionDefinitionNode;
struct ScopeNode;
struct VariableDeclarationNode;
struct VariableAccessNode;
struct UnaryOperatorNode;
struct BinaryOperatorNode;
struct StructNode;
struct FunctionCallNode;
struct BindNode;
struct IfNode;
struct WhileNode;
struct LiteralNode;
struct ReturnNode;
}

class ASTPrinter {
public:
    static void print(const AST::Node* node, int indent = 0);
    static void print(const AST::FileNode*, int indent = 0);

private:
    static void print(const AST::FunctionDefinitionNode*, int indent = 0);
    static void print(const AST::ScopeNode*, int indent = 0);
    static void print(const AST::VariableDeclarationNode*, int indent = 0);
    static void print(const AST::VariableAccessNode*, int indent = 0);
    static void print(const AST::UnaryOperatorNode*, int indent = 0);
    static void print(const AST::BinaryOperatorNode*, int indent = 0);
    static void print(const AST::StructNode*, int indent = 0);
    static void print(const AST::FunctionCallNode*, int indent = 0);
    static void print(const AST::BindNode*, int indent = 0);
    static void print(const AST::IfNode*, int indent = 0);
    static void print(const AST::WhileNode*, int indent = 0);
    static void print(const AST::LiteralNode*, int indent = 0);
    static void print(const AST::ReturnNode*, int indent = 0);
};