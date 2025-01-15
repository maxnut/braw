#pragma once

#include "parsernew/nodes/node.hpp"

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
struct ImportNode;
}

class ASTPrinter {
public:
    static void print(std::shared_ptr<AST::Node> node, int indent = 0);
    static void print(std::shared_ptr<AST::FileNode>, int indent = 0);

private:
    static void print(std::shared_ptr<AST::FunctionDefinitionNode>, int indent = 0);
    static void print(std::shared_ptr<AST::ScopeNode>, int indent = 0);
    static void print(std::shared_ptr<AST::VariableDeclarationNode>, int indent = 0);
    static void print(std::shared_ptr<AST::VariableAccessNode>, int indent = 0);
    static void print(std::shared_ptr<AST::UnaryOperatorNode>, int indent = 0);
    static void print(std::shared_ptr<AST::BinaryOperatorNode>, int indent = 0);
    static void print(std::shared_ptr<AST::StructNode>, int indent = 0);
    static void print(std::shared_ptr<AST::FunctionCallNode>, int indent = 0);
    static void print(std::shared_ptr<AST::BindNode>, int indent = 0);
    static void print(std::shared_ptr<AST::IfNode>, int indent = 0);
    static void print(std::shared_ptr<AST::WhileNode>, int indent = 0);
    static void print(std::shared_ptr<AST::LiteralNode>, int indent = 0);
    static void print(std::shared_ptr<AST::ReturnNode>, int indent = 0);
    static void print(std::shared_ptr<AST::ImportNode>, int indent = 0);
};