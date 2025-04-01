#pragma once

#include "braw_context.hpp"
#include "parser/nodes/function_definition.hpp"

#include <string>
#include <unordered_set>
#include <utility>
#include <cstdint>
#include <expected>
#include <optional>

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
struct ForNode;
}

struct SemanticError {
    std::string m_message;
    std::filesystem::path m_path;
    std::pair<uint32_t, uint32_t> m_rangeBegin;
    std::pair<uint32_t, uint32_t> m_rangeEnd;
};

class SemanticAnalyzer {
public:
    static std::optional<SemanticError> analyze(const AST::FileNode*, BrawContext&);
    static std::expected<BrawContext, SemanticError> fillTypes(const AST::FileNode*);

private:
    static std::optional<SemanticError> fillTypes(const AST::FileNode*, BrawContext&);
    static std::optional<SemanticError> analyze(AST::Node*, BrawContext&);
    static std::optional<SemanticError> analyze(const AST::FunctionDefinitionNode*, BrawContext&);
    static std::optional<SemanticError> analyze(const AST::ScopeNode*, BrawContext&);
    static std::optional<SemanticError> analyze(AST::VariableDeclarationNode*, BrawContext&);
    static std::optional<SemanticError> analyze(const AST::VariableAccessNode*, BrawContext&);
    static std::optional<SemanticError> analyze(const AST::UnaryOperatorNode*, BrawContext&);
    static std::optional<SemanticError> analyze(const AST::BinaryOperatorNode*, BrawContext&);
    static std::optional<SemanticError> analyze(const AST::StructNode*, BrawContext&);
    static std::optional<SemanticError> analyze(const AST::FunctionCallNode*, BrawContext&);
    static std::optional<SemanticError> analyze(const AST::IfNode*, BrawContext&);
    static std::optional<SemanticError> analyze(const AST::WhileNode*, BrawContext&);
    static std::optional<SemanticError> analyze(const AST::ForNode*, BrawContext&);
    static std::optional<SemanticError> analyze(const AST::LiteralNode*, BrawContext&);
    static std::optional<SemanticError> analyze(const AST::ReturnNode*, BrawContext&);

    static std::optional<TypeInfo> getType(const AST::Node* node, BrawContext& ctx);
    static bool hasOperator(const TypeInfo& type, const std::string& operatorName);

    static SemanticError unknownType(const AST::Node* causer, const std::string& type, BrawContext& ctx);
    static SemanticError mismatchedTypes(const AST::Node* causer, const std::string& type1, const std::string& type2, BrawContext& ctx);
    static SemanticError duplicateFunction(const AST::Node* causer, const AST::FunctionSignature& signature, BrawContext& ctx);
    static SemanticError missingReturn(const AST::Node* causer, const AST::FunctionSignature& signature, BrawContext& ctx);
    static SemanticError unknownVariable(const AST::VariableAccessNode* causer, BrawContext& ctx);
    static SemanticError unknownOperator(const AST::UnaryOperatorNode* causer, BrawContext& ctx);
    static SemanticError unknownOperator(const AST::BinaryOperatorNode* causer, BrawContext& ctx);
    static SemanticError unknownFunction(const AST::FunctionCallNode* causer, const std::vector<TypeInfo>& types, BrawContext& ctx);
    static SemanticError unknownMember(const AST::Node* causer, const std::string& type, const std::string& member, BrawContext& ctx);
    static SemanticError invalidCast(const AST::UnaryOperatorNode* causer, const std::string& type, BrawContext& ctx);
    static SemanticError invalidInstruction(const AST::Node* causer, const AST::Node* origin, BrawContext& ctx);
    static SemanticError cannotInferType(const AST::VariableDeclarationNode* causer, BrawContext& ctx);

    static std::unordered_set<AST::Node::Type> expressionWhitelist;

    friend class IRBuilder;
};