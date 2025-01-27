#pragma once

#include "ir/operand.hpp"
#include "ir/register.hpp"
#include "parser/nodes/file.hpp"
#include "ir/file.hpp"
#include "parser/nodes/node.hpp"
#include "braw_context.hpp"

#include <unordered_map>
#include <vector>
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

struct RegisterInfo {
    std::string m_type;
    std::string m_name;
};

typedef std::vector<std::unique_ptr<Instruction>> Instructions;

struct IRFunctionContext {
    std::unordered_map<std::string, std::shared_ptr<Register>> m_registers;
    Instructions m_instructions;
    uint32_t m_scopeDepth = 0;
};

class IRBuilder {
public:
    static std::vector<File> build(const AST::FileNode* root, BrawContext& context);
private:
    static void build(const AST::Node* node, BrawContext& context, IRFunctionContext& ictx);

    static Function build(const AST::FunctionDefinitionNode* node, BrawContext& context);
    static void build(const AST::ScopeNode* node, BrawContext& context, IRFunctionContext& ictx);
    static void build(const AST::VariableDeclarationNode* node, BrawContext& context, IRFunctionContext& ictx);
    static void build(const AST::IfNode* node, BrawContext& context, IRFunctionContext& ictx);
    static void build(const AST::ReturnNode* node, BrawContext& context, IRFunctionContext& ictx);
    static Operand buildCall(const AST::FunctionCallNode* node, BrawContext& context, IRFunctionContext& ictx);
    static Operand buildExpression(const AST::Node* node, BrawContext& context, IRFunctionContext& ictx);
    static Operand buildBinaryOperator(const AST::BinaryOperatorNode* node, BrawContext& context, IRFunctionContext& ictx);
    static void buildAssignment(const AST::BinaryOperatorNode* node, BrawContext& context, IRFunctionContext& ictx);

    static TypeInfo getOperandType(Operand op, BrawContext& context, IRFunctionContext& ictx);

    static void moveToRegister(const std::string& name, Operand& op, BrawContext& context, IRFunctionContext& ictx);
    static std::shared_ptr<Register> makeOrGetRegister(const std::string& name, IRFunctionContext& ictx);
    static RegisterType getRegisterType(const TypeInfo& type);
};