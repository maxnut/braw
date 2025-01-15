#pragma once

#include "parser/nodes/file.hpp"
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

struct AddressNode;
struct ArrowNode;
struct AssignmentNode;
struct BinaryOperatorNode;
struct CastNode;
struct DereferenceNode;
struct DotNode;
struct EvaluatableNode;
struct FunctionCallNode;
struct FunctionDefinitionNode;
struct FunctionInstructionNode;
struct IfNode;
struct LiteralNode;
struct NativeFunctionCallNode;
struct NativeFunctionNode;
struct ReturnNode;
struct ScopeNode;
struct VariableAccessNode;
struct VariableDeclarationNode;
struct WhileNode;

class ETBuilder {
public:
    static std::shared_ptr<FileNode> build(const AST::FileNode* ast, BrawContext& context);

public:
    static std::shared_ptr<AddressNode> buildAddress(const AST::UnaryOperatorNode* ast, BrawContext& context);
    static std::shared_ptr<ArrowNode> buildArrow(const AST::UnaryOperatorNode* ast, BrawContext& context);
    static std::shared_ptr<AssignmentNode> buildAssignment(const AST::BinaryOperatorNode* ast, BrawContext& context);
    static std::shared_ptr<BinaryOperatorNode> buildBinaryOperator(const AST::BinaryOperatorNode* ast, BrawContext& context);
    static std::shared_ptr<CastNode> buildCast(const AST::UnaryOperatorNode* ast, BrawContext& context);
    static std::shared_ptr<DereferenceNode> buildDereference(const AST::UnaryOperatorNode* ast, BrawContext& context);
    static std::shared_ptr<DotNode> buildDot(const AST::UnaryOperatorNode* ast, BrawContext& context);
    static std::shared_ptr<FunctionCallNode> buildFunctionCall(const AST::UnaryOperatorNode* ast, BrawContext& context);
    static std::shared_ptr<FunctionDefinitionNode> buildFunctionDefinition(const AST::UnaryOperatorNode* ast, BrawContext& context);
    static std::shared_ptr<IfNode> buildIf(const AST::UnaryOperatorNode* ast, BrawContext& context);
    static std::shared_ptr<LiteralNode> buildLiteral(const AST::UnaryOperatorNode* ast, BrawContext& context);
    static std::shared_ptr<NativeFunctionCallNode> buildNativeFunctionCall(const AST::UnaryOperatorNode* ast, BrawContext& context);
    static std::shared_ptr<ReturnNode> buildReturn(const AST::UnaryOperatorNode* ast, BrawContext& context);
    static std::shared_ptr<ScopeNode> buildScope(const AST::UnaryOperatorNode* ast, BrawContext& context);
    static std::shared_ptr<VariableAccessNode> buildVariableAccess(const AST::UnaryOperatorNode* ast, BrawContext& context);
    static std::shared_ptr<VariableDeclarationNode> buildVariableDeclaration(const AST::UnaryOperatorNode* ast, BrawContext& context);
    static std::shared_ptr<WhileNode> buildWhile(const AST::UnaryOperatorNode* ast, BrawContext& context);

    static std::vector<std::shared_ptr<NativeFunctionNode>> buildBind(const AST::BindNode* ast, BrawContext& context);
};