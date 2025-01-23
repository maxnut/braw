#pragma once

#include "ir/label.hpp"
#include "ir/operator.hpp"
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
typedef std::unordered_map<std::string, RegisterInfo> RegisterTable;

struct IRFunctionContext {
    std::vector<RegisterTable> m_tables;
    Instructions m_instructions;

    RegisterInfo getRegisterInfo(const std::string& name) const {
        for(int i = m_tables.size() - 1; i >= 0; i--) {
            auto it = m_tables[i].find(name);
            if(it != m_tables[i].end())
                return it->second;
        }
        return {};
    }
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
    static Operator buildCall(const AST::FunctionCallNode* node, BrawContext& context, IRFunctionContext& ictx);
    static Operator buildExpression(const AST::Node* node, BrawContext& context, IRFunctionContext& ictx);
    static Operator buildBinaryOperator(const AST::BinaryOperatorNode* node, BrawContext& context, IRFunctionContext& ictx);
    static void buildAssignment(const AST::BinaryOperatorNode* node, BrawContext& context, IRFunctionContext& ictx);

    static TypeInfo getOperatorType(Operator op, BrawContext& context, IRFunctionContext& ictx);
};