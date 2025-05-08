#pragma once

#include "braw_context.hpp"
#include "ir/instruction.hpp"
#include "parser/nodes/binary_operator.hpp"
#include "parser/nodes/file.hpp"
#include "parser/nodes/for.hpp"
#include "parser/nodes/function_call.hpp"
#include "parser/nodes/if.hpp"
#include "parser/nodes/return.hpp"
#include "parser/nodes/unary_operator.hpp"
#include "parser/nodes/while.hpp"
#include "ssa/file.hpp"
#include "ssa/instruction.hpp"
#include "ssa/operand.hpp"
#include "ssa/operation.hpp"
#include "block.hpp"
#include <memory>
#include <unordered_map>
#include <unordered_set>
namespace SSA {

struct FunctionContext {
    std::unordered_map<std::string, std::shared_ptr<Register>> m_registers;
    std::vector<std::shared_ptr<Instruction>> m_instructions;
    std::vector<uint64_t> m_scopeIdStack;
    std::shared_ptr<Register> m_returnRegister;
    Function* m_function;
};


class Builder {
public:
    static std::vector<File> build(AST::FileNode* file, BrawContext& ctx);
    static void build(AST::Node* node, BrawContext& context, FunctionContext& ictx);
    static Function build(const AST::FunctionDefinitionNode* node, BrawContext& context);
    static void build(AST::ScopeNode* node, BrawContext& context, FunctionContext& ictx);
    static void build(AST::VariableDeclarationNode* node, BrawContext& context, FunctionContext& ictx);
    static void build(AST::IfNode* node, BrawContext& context, FunctionContext& ictx);
    static void build(AST::WhileNode* node, BrawContext& context, FunctionContext& ictx);
    static void build(AST::ForNode* node, BrawContext& context, FunctionContext& ictx);
    static void build(AST::ReturnNode* node, BrawContext& context, FunctionContext& ictx);
    static std::shared_ptr<Operand> buildCall(AST::FunctionCallNode* node, BrawContext& context, FunctionContext& ictx);
    static void buildAssignment(AST::BinaryOperatorNode* node, BrawContext& context, FunctionContext& ictx);
    static std::shared_ptr<Operand> buildExpression(AST::Node* node, BrawContext& context, FunctionContext& ictx);
    static std::shared_ptr<Operand> buildBinaryOperator(AST::BinaryOperatorNode* node, BrawContext& context, FunctionContext& ictx);
    static std::shared_ptr<Operand> buildUnaryOperator(AST::UnaryOperatorNode* node, BrawContext& context, FunctionContext& ictx);
    static std::shared_ptr<Operand> dotOperator(AST::UnaryOperatorNode* node, std::shared_ptr<Operand> op, BrawContext& context, FunctionContext& ictx);
    static std::shared_ptr<Operand> dereferenceOperator(AST::UnaryOperatorNode* node, std::shared_ptr<Operand> op, BrawContext& context, FunctionContext& ictx);
    static std::shared_ptr<Operand> addressOperator(AST::UnaryOperatorNode* node, std::shared_ptr<Operand> op, BrawContext& context, FunctionContext& ictx);
    static std::shared_ptr<Operand> subscriptOperator(AST::UnaryOperatorNode* node, std::shared_ptr<Operand> op, BrawContext& context, FunctionContext& ictx);
    static std::shared_ptr<Operand> castOperator(AST::UnaryOperatorNode* node, std::shared_ptr<Operand> op, BrawContext& context, FunctionContext& ictx);
    static std::shared_ptr<Operand> logicalNotOperator(AST::UnaryOperatorNode* node, std::shared_ptr<Operand> op, BrawContext& context, FunctionContext& ictx);

    static std::shared_ptr<Register> makeOrGetRegister(const std::string& name, FunctionContext& ctx, bool useScopeDepth = false);
    static void assign(std::shared_ptr<Operand> to, std::shared_ptr<Operation> operation, std::pair<uint32_t, uint32_t> pos, FunctionContext& ctx);
    static std::shared_ptr<Operation> point(std::shared_ptr<Operand> op, FunctionContext& ictx);
    static std::shared_ptr<Operation> load(std::shared_ptr<Operand> op, FunctionContext& ictx);

    static std::vector<std::shared_ptr<Block>> buildCFG(Function& f, FunctionContext& context);
    static std::vector<std::shared_ptr<Block>> getBlocks(const Function& f);
    static void buildGraphRecursive(std::shared_ptr<Block> root, const std::unordered_map<size_t, size_t>& blockForInstruction, const std::vector<std::shared_ptr<Block>>& blocks, std::unordered_set<std::shared_ptr<Block>>& visited, const Function& f);
    static void placePhiBlocks(std::shared_ptr<Operand> op, std::vector<std::shared_ptr<Block>> blocks, const std::vector<std::shared_ptr<Block>>& allBlocks, Function& f);
    static void rename(std::shared_ptr<Block> block, Function& f, std::unordered_map<std::string, size_t>& counters, std::unordered_map<std::string, std::vector<std::string>>& nameStack, std::unordered_set<std::shared_ptr<Block>>& visited, std::unordered_map<std::string, std::shared_ptr<Operand>>& nameForOperand, FunctionContext& context);

    static std::string operandString(std::shared_ptr<Operand> op);
    static std::shared_ptr<Operation> operation(Operation::Type t, const TypeInfo& ti, FunctionContext& ictx, std::shared_ptr<Operand> o1, std::shared_ptr<Operand> o2 = nullptr);
};

}