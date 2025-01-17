#pragma once

#include "parser/nodes/file.hpp"
#include "execution-tree/nodes/evaluatable.hpp"
#include "execution-tree/nodes/file.hpp"
#include "braw_context.hpp"

#include <memory>
#include <vector>

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

struct ScopeNode;

class ETBuilder {
public:
    static std::shared_ptr<FileNode> buildFile(const AST::FileNode* ast, BrawContext& context);

public:
    static std::unique_ptr<FunctionInstructionNode> buildInstruction(const AST::Node* ast, BrawContext& context);
    static std::unique_ptr<EvaluatableNode> buildEvaluatable(const AST::Node* ast, BrawContext& context);
    static std::unique_ptr<EvaluatableNode> buildUnaryOperator(const AST::UnaryOperatorNode* ast, BrawContext& context);
    static std::unique_ptr<EvaluatableNode> buildAddress(const AST::UnaryOperatorNode* ast, BrawContext& context);
    static std::unique_ptr<EvaluatableNode> buildArrow(const AST::UnaryOperatorNode* ast, BrawContext& context);
    static std::unique_ptr<FunctionInstructionNode> buildAssignment(const AST::BinaryOperatorNode* ast, BrawContext& context);
    static std::unique_ptr<EvaluatableNode> buildBinaryOperator(const AST::BinaryOperatorNode* ast, BrawContext& context);
    static std::unique_ptr<EvaluatableNode> buildCast(const AST::UnaryOperatorNode* ast, BrawContext& context);
    static std::unique_ptr<EvaluatableNode> buildDereference(const AST::UnaryOperatorNode* ast, BrawContext& context);
    static std::unique_ptr<EvaluatableNode> buildDot(const AST::UnaryOperatorNode* ast, BrawContext& context);
    static std::unique_ptr<EvaluatableNode> buildFunctionCall(const AST::FunctionCallNode* ast, BrawContext& context);
    static std::shared_ptr<FunctionDefinitionNode> buildFunctionDefinition(const AST::FunctionDefinitionNode* ast, BrawContext& context);
    static std::unique_ptr<FunctionInstructionNode> buildIf(const AST::IfNode* ast, BrawContext& context);
    static std::unique_ptr<EvaluatableNode> buildLiteral(const AST::LiteralNode* ast, BrawContext& context);
    static std::unique_ptr<FunctionInstructionNode> buildReturn(const AST::ReturnNode* ast, BrawContext& context);
    static std::unique_ptr<ScopeNode> buildScope(const AST::ScopeNode* ast, BrawContext& context);
    static std::unique_ptr<EvaluatableNode> buildVariableAccess(const AST::VariableAccessNode* ast, BrawContext& context);
    static std::unique_ptr<FunctionInstructionNode> buildVariableDeclaration(const AST::VariableDeclarationNode* ast, BrawContext& context);
    static std::unique_ptr<FunctionInstructionNode> buildWhile(const AST::WhileNode* ast, BrawContext& context);

    static std::vector<std::shared_ptr<NativeFunctionNode>> buildBind(const AST::BindNode* ast, BrawContext& context);
};