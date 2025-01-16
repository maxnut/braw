#pragma once

#include "stack.hpp"
#include "function_context.hpp"

class FunctionDefinitionNode;
class ScopeNode;
class FunctionInstructionNode;
class EvaluatableNode;
class AssignmentNode;
class FunctionCallNode;
class BinaryOperatorNode;
class NativeFunctionCallNode;
class IfNode;
class WhileNode;
class LiteralNode;
class NativeFunctionNode;
class ReturnNode;
class VariableAccessNode;
class VariableDeclarationNode;
class AddressNode;
class DereferenceNode;
class DotNode;
class ArrowNode;
class CastNode;

class Interpreter {
public:
    void invokeFunction(FunctionDefinitionNode* function, Stack& stack, Memory* returnValue, size_t functionPtr);
    void invokeNativeFunction(NativeFunctionNode* function, Stack& stack, Memory* returnValue, size_t functionPtr);
    void executeScope(ScopeNode* scope, Stack& stack, FunctionContext& context);

    void visitAssignment(AssignmentNode* instruction, Stack& stack, FunctionContext& context);
    void visitIf(IfNode* instruction, Stack& stack, FunctionContext& context);
    void visitWhile(WhileNode* instruction, Stack& stack, FunctionContext& context);
    void visitReturn(ReturnNode* instruction, Stack& stack, FunctionContext& context);
    void visitVariableDeclaration(VariableDeclarationNode* instruction, Stack& stack, FunctionContext& context);
    Memory visitBinaryOperator(BinaryOperatorNode* instruction, Stack& stack, FunctionContext& context);
    Memory visitFunctionCall(FunctionCallNode* instruction, Stack& stack, FunctionContext& context);
    Memory visitNativeFunctionCall(NativeFunctionCallNode* instruction, Stack& stack, FunctionContext& context);
    Memory visitLiteral(LiteralNode* instruction, Stack& stack, FunctionContext& context);
    Memory visitVariableAccess(VariableAccessNode* instruction, Stack& stack, FunctionContext& context);
    Memory visitAddress(AddressNode* instruction, Stack& stack, FunctionContext& context);
    Memory visitDereference(DereferenceNode* instruction, Stack& stack, FunctionContext& context);
    Memory visitDot(DotNode* instruction, Stack& stack, FunctionContext& context);
    Memory visitArrow(ArrowNode* instruction, Stack& stack, FunctionContext& context);
    Memory visitCast(CastNode* instruction, Stack& stack, FunctionContext& context);
};