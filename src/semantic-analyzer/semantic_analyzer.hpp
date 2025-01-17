#pragma once

#include "braw_context.hpp"
#include "parser/nodes/function_definition.hpp"

#include <string>
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
}

struct SemanticError {
    std::string m_message;
    std::pair<uint32_t, uint32_t> m_rangeBegin;
    std::pair<uint32_t, uint32_t> m_rangeEnd;
};

class SemanticAnalyzer {
public:
    static std::expected<BrawContext, SemanticError> analyze(const AST::FileNode*);

private:
    static std::optional<SemanticError> analyze(const AST::Node*, BrawContext&);
    static std::optional<SemanticError> analyze(const AST::FileNode*, BrawContext&);
    static std::optional<SemanticError> analyze(const AST::FunctionDefinitionNode*, BrawContext&);
    static std::optional<SemanticError> analyze(const AST::ScopeNode*, BrawContext&);
    static std::optional<SemanticError> analyze(const AST::VariableDeclarationNode*, BrawContext&);
    static std::optional<SemanticError> analyze(const AST::VariableAccessNode*, BrawContext&);
    static std::optional<SemanticError> analyze(const AST::UnaryOperatorNode*, BrawContext&);
    static std::optional<SemanticError> analyze(const AST::BinaryOperatorNode*, BrawContext&);
    static std::optional<SemanticError> analyze(const AST::StructNode*, BrawContext&);
    static std::optional<SemanticError> analyze(const AST::FunctionCallNode*, BrawContext&);
    static std::optional<SemanticError> analyze(const AST::BindNode*, BrawContext&);
    static std::optional<SemanticError> analyze(const AST::IfNode*, BrawContext&);
    static std::optional<SemanticError> analyze(const AST::WhileNode*, BrawContext&);
    static std::optional<SemanticError> analyze(const AST::LiteralNode*, BrawContext&);
    static std::optional<SemanticError> analyze(const AST::ReturnNode*, BrawContext&);

    static std::optional<TypeInfo> getType(const AST::Node* node, BrawContext& ctx);

    static SemanticError unknownType(const AST::Node* causer, const std::string& type);
    static SemanticError mismatchedTypes(const AST::Node* causer, const std::string& type1, const std::string& type2);
    static SemanticError duplicateFunction(const AST::Node* causer, const AST::FunctionSignature& signature);
    static SemanticError unknownVariable(const AST::VariableAccessNode* causer);
    static SemanticError unknownOperator(const AST::UnaryOperatorNode* causer);
    static SemanticError unknownOperator(const AST::BinaryOperatorNode* causer);
    static SemanticError unknownFunction(const AST::FunctionCallNode* causer, const std::vector<TypeInfo>& types);
    static SemanticError unknownMember(const AST::Node* causer, const std::string& type, const std::string& member);
    static SemanticError invalidCast(const AST::UnaryOperatorNode* causer, const std::string& type);
};