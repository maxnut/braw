#include "semantic_analyzer.hpp"
#include "parser/nodes/continue.hpp"
#include "parser/nodes/file.hpp"
#include "parser/nodes/for.hpp"
#include "parser/nodes/function_definition.hpp"
#include "parser/nodes/scope.hpp"
#include "parser/nodes/variable_declaration.hpp"
#include "parser/nodes/variable_access.hpp"
#include "parser/nodes/unary_operator.hpp"
#include "parser/nodes/binary_operator.hpp"
#include "parser/nodes/struct.hpp"
#include "parser/nodes/function_call.hpp"
#include "parser/nodes/if.hpp"
#include "parser/nodes/while.hpp"
#include "parser/nodes/literal.hpp"
#include "parser/nodes/return.hpp"
#include "rules.hpp"
#include "utils.hpp"

#include <optional>
#include <spdlog/fmt/fmt.h>

std::expected<BrawContext, SemanticError> SemanticAnalyzer::fillTypes(const AST::FileNode* file) {
    BrawContext ctx;
    std::optional<SemanticError> errorOpt = fillTypes(file, ctx);

    if(errorOpt)
        return std::unexpected{errorOpt.value()};
    return ctx;
}

std::optional<SemanticError> SemanticAnalyzer::fillTypes(const AST::FileNode* file, BrawContext& context) {
    for(auto import : file->m_imports) {
        auto errOpt = fillTypes(import.get(), context);
        if(errOpt) return errOpt;
    }
    for(auto sstruct : file->m_structs) {
        auto errOpt = analyze(sstruct.get(), context);
        if(errOpt) return errOpt;
    }
    return std::nullopt;
}

std::optional<SemanticError> SemanticAnalyzer::analyze(AST::Node* root, BrawContext& context) {
    switch (root->m_type) {
        case AST::Node::File:
            return analyze(static_cast<const AST::FileNode*>(root), context);
        case AST::Node::FunctionDefinition:
            return analyze(static_cast<const AST::FunctionDefinitionNode*>(root), context);
        case AST::Node::Scope:
            return analyze(static_cast<const AST::ScopeNode*>(root), context);
        case AST::Node::VariableDeclaration:
            return analyze(static_cast<AST::VariableDeclarationNode*>(root), context);
        case AST::Node::VariableAccess:
            return analyze(static_cast<AST::VariableAccessNode*>(root), context);
        case AST::Node::UnaryOperator:
            return analyze(static_cast<const AST::UnaryOperatorNode*>(root), context);
        case AST::Node::BinaryOperator:
            return analyze(static_cast<const AST::BinaryOperatorNode*>(root), context);
        case AST::Node::Struct:
            return analyze(static_cast<const AST::StructNode*>(root), context);
        case AST::Node::FunctionCall:
            return analyze(static_cast<const AST::FunctionCallNode*>(root), context);
        case AST::Node::If:
            return analyze(static_cast<const AST::IfNode*>(root), context);
        case AST::Node::While:
            return analyze(static_cast<const AST::WhileNode*>(root), context);
        case AST::Node::For:
            return analyze(static_cast<const AST::ForNode*>(root), context);
        case AST::Node::Literal:
            return analyze(static_cast<const AST::LiteralNode*>(root), context);
        case AST::Node::Return:
            return analyze(static_cast<const AST::ReturnNode*>(root), context);
        case AST::Node::Break: {
            if(!context.m_loopOrSwitch) return notInLoop(static_cast<const AST::BreakNode*>(root), context);
            return std::nullopt;
        }
        case AST::Node::Continue: {
            if(!context.m_loopOrSwitch) return notInLoop(static_cast<const AST::ContinueNode*>(root), context);
            return std::nullopt;
        }
        case AST::Node::Macro:
        case AST::Node::MacroParameterReference:
        case AST::Node::MacroParameter:
        case AST::Node::MacroCall:
        case AST::Node::MacroIf:
        case AST::Node::MacroForeach:
        case AST::Node::Identifier:
          break;
        }
    return SemanticError("Unexpected node type");
}

//TODO: add error handling
std::expected<TypeInfo, SemanticError> SemanticAnalyzer::getType(const AST::Node* node, BrawContext& ctx) {
    switch (node->m_type) {
        case AST::Node::VariableDeclaration: {
            auto optType = ctx.getTypeInfo(Utils::getIdentifier(static_cast<const AST::VariableDeclarationNode*>(node)->m_type));
            if(!optType) return std::unexpected{unknownType(node, Utils::getIdentifier(static_cast<const AST::VariableDeclarationNode*>(node)->m_type), ctx)};
            return optType.value();
        }
        case AST::Node::VariableAccess: {
            const std::string& var = Utils::getIdentifier(static_cast<const AST::VariableAccessNode*>(node)->m_name);
            auto optScope = ctx.getScopeInfo(var);
            if(!optScope) return std::unexpected{unknownVariable(static_cast<const AST::VariableAccessNode*>(node), ctx)};
            return optScope.value().m_type;
        }
        case AST::Node::UnaryOperator: {
            const AST::UnaryOperatorNode* op = static_cast<const AST::UnaryOperatorNode*>(node);
            auto typeOr = getType(op->m_operand.get(), ctx);
            if(!typeOr) return typeOr;
            TypeInfo type = typeOr.value();
            if(op->m_operator == "->") {
                auto typeOpt = Utils::getRawType(type, ctx);
                if(!typeOpt) return std::unexpected{unknownType(op->m_operand.get(), type.m_name, ctx)};
                type = typeOpt.value();
            }

            if(op->m_operator == "&")
                return Utils::makePointer(type);
            else if(op->m_operator == "*" || op->m_operator == "[]") {
                auto typeOpt = Utils::getRawType(type, ctx);
                if(!typeOpt) return std::unexpected{unknownType(op->m_operand.get(), type.m_name, ctx)};
                return typeOpt.value();
            }
            else if(op->m_operator == "cast") {
                auto typeOpt = ctx.getTypeInfo(Utils::getIdentifier(op->m_data));
                if(!typeOpt) return std::unexpected{unknownType(op->m_operand.get(), type.m_name, ctx)};
                return typeOpt.value();
            }
            else if(op->m_operator == "." || op->m_operator == "->") {
                auto typeOpt = ctx.getTypeInfo(type.m_members.at(Utils::getIdentifier(op->m_data)).m_type);
                if(!typeOpt) return std::unexpected{unknownType(op->m_operand.get(), type.m_name, ctx)};
                return typeOpt.value();
            }
            else if(op->m_operator == "!") {
                return ctx.getTypeInfo(BOOL_T).value();
            }
            else if(op->m_operator == "pre++" || op->m_operator == "pre--" || op->m_operator == "post++" || op->m_operator == "post--") {
                return type;
            }
            return std::unexpected{unknownOperator(op, ctx)};
        }
        case AST::Node::BinaryOperator: {
            const AST::BinaryOperatorNode* op = static_cast<const AST::BinaryOperatorNode*>(node);
            auto leftOpt = getType(op->m_left.get(), ctx);
            if(!leftOpt) return leftOpt;
            auto rightOpt = getType(op->m_right.get(), ctx);
            if(!rightOpt) return rightOpt;
            if(!hasOperator(leftOpt.value(), op->m_operator)) return std::unexpected(unknownOperator(op, ctx));

            if(Rules::isPtr(leftOpt->m_name))
                return leftOpt.value();

            auto typeOpt = ctx.getTypeInfo(leftOpt->m_operators[op->m_operator].m_returnType);
            if(!typeOpt) return std::unexpected{unknownType(op->m_left.get(), leftOpt.value().m_name, ctx)};
            return typeOpt.value();
        }
        case AST::Node::FunctionCall: {
            const AST::FunctionCallNode* call = static_cast<const AST::FunctionCallNode*>(node);
            std::vector<TypeInfo> params;
            params.reserve(call->m_parameters.size());
            for(auto& param : call->m_parameters) {
                auto errOpt = analyze(param.get(), ctx);
                if(errOpt) return std::unexpected{errOpt.value()};
                auto typeOr = getType(param.get(), ctx);
                if(!typeOr) return std::unexpected{typeOr.error()};
                params.push_back(typeOr.value());
            }
            auto funcOpt = ctx.getFunction(Utils::getIdentifier(call->m_name), params);
            if(!funcOpt) return std::unexpected{unknownFunction(call, params, ctx)};
            return funcOpt->m_returnType;
        }
        case AST::Node::Literal: {
            const AST::LiteralNode* literal = static_cast<const AST::LiteralNode*>(node);
            switch(literal->m_value.index()) {
                case 0: return ctx.m_typeTable[INT_T];
                case 1: return ctx.m_typeTable[LONG_T];
                case 2: return ctx.m_typeTable[FLOAT_T];
                case 3: return ctx.m_typeTable[DOUBLE_T];
                case 4: return ctx.m_typeTable[BOOL_T];
                case 5: return Utils::makePointer(ctx.m_typeTable[CHAR_T]);
                case 6: return ctx.m_typeTable[CHAR_T];
                case 7: return Utils::makePointer(ctx.m_typeTable[VOID_T]);
            }
        }
        default:
            break;
    }

    return std::unexpected{SemanticError("How")};
}

SemanticError SemanticAnalyzer::unknownType(const AST::Node* causer, const std::string& type, BrawContext& ctx) {
    return SemanticError("Unknown type: " + type, ctx.m_currentFile, causer->m_rangeBegin, causer->m_rangeEnd);
}

SemanticError SemanticAnalyzer::mismatchedTypes(const AST::Node* causer, const std::string& type1, const std::string& type2, BrawContext& ctx) {
    return SemanticError(
        fmt::format("Mismatched types: {} and {}", type1, type2),
        ctx.m_currentFile,
        causer->m_rangeBegin,
        causer->m_rangeEnd
    );
}

SemanticError SemanticAnalyzer::duplicateFunction(const AST::Node* causer, const AST::FunctionSignature& signature, BrawContext& ctx) {
    return SemanticError(
        fmt::format("Function {} already defined", Utils::functionSignatureString(signature)),
        ctx.m_currentFile,
        causer->m_rangeBegin,
        causer->m_rangeEnd
    );
}

SemanticError SemanticAnalyzer::missingReturn(const AST::Node* causer, const AST::FunctionSignature& signature, BrawContext& ctx) {
    return SemanticError(
        fmt::format("Function {} doesn't return a value", Utils::functionSignatureString(signature)),
        ctx.m_currentFile,
        causer->m_rangeBegin,
        causer->m_rangeEnd
    );
}

SemanticError SemanticAnalyzer::unknownVariable(const AST::VariableAccessNode* causer, BrawContext& ctx) {
    return SemanticError(
        fmt::format("Unknown variable: {}", Utils::getIdentifier(causer->m_name)),
        ctx.m_currentFile,
        causer->m_rangeBegin,
        causer->m_rangeEnd
    );
}

SemanticError SemanticAnalyzer::unknownOperator(const AST::UnaryOperatorNode* causer, BrawContext& ctx) {
    if(Utils::getIdentifier(causer->m_data).size() > 0) {
        return SemanticError(
            fmt::format("Unknown operator: {} ({})", causer->m_operator, Utils::getIdentifier(causer->m_data)),
            ctx.m_currentFile,
            causer->m_rangeBegin,
            causer->m_rangeEnd
        );
    }

    return SemanticError(
        fmt::format("Unknown operator: {}", causer->m_operator),
        ctx.m_currentFile,
        causer->m_rangeBegin,
        causer->m_rangeEnd
    );
}

SemanticError SemanticAnalyzer::unknownOperator(const AST::BinaryOperatorNode* causer, BrawContext& ctx) {
    return SemanticError(
        fmt::format("Unknown operator: {}", causer->m_operator),
        ctx.m_currentFile,
        causer->m_rangeBegin,
        causer->m_rangeEnd
    );
}

SemanticError SemanticAnalyzer::unknownFunction(const AST::FunctionCallNode* causer, const std::vector<TypeInfo>& parameters, BrawContext& ctx) {
    std::string parameterString = "";
    for(int i = 0; i < causer->m_parameters.size(); i++) {
        parameterString += parameters[i].m_name;
        if(i < causer->m_parameters.size() - 1) parameterString += ", ";
    }

    return SemanticError(
        fmt::format("Unknown function: {}({})", Utils::getIdentifier(causer->m_name), parameterString),
        ctx.m_currentFile,
        causer->m_rangeBegin,
        causer->m_rangeEnd
    );
}

SemanticError SemanticAnalyzer::unknownMember(const AST::Node* causer, const std::string& type, const std::string& member, BrawContext& ctx) {
    return SemanticError(
        fmt::format("Unknown member {} in {}", member, type),
        ctx.m_currentFile,
        causer->m_rangeBegin,
        causer->m_rangeEnd
    );
}

SemanticError SemanticAnalyzer::invalidCast(const AST::UnaryOperatorNode* causer, const std::string& type, BrawContext& ctx) {
    return SemanticError(
        fmt::format("Invalid cast from {} to {}", type, Utils::getIdentifier(causer->m_data)),
        ctx.m_currentFile,
        causer->m_rangeBegin,
        causer->m_rangeEnd
    );
}

SemanticError SemanticAnalyzer::invalidOperator(const AST::UnaryOperatorNode* causer, BrawContext& ctx) {
    return SemanticError(
        fmt::format("Invalid operator {}", causer->m_operator),
        ctx.m_currentFile,
        causer->m_rangeBegin,
        causer->m_rangeEnd
    );
}


SemanticError SemanticAnalyzer::invalidInstruction(const AST::Node* causer, const AST::Node* origin, BrawContext& ctx) {
    return SemanticError(
        fmt::format("This instruction cannot be used at {}", Utils::extractRangeWithContext(ctx.m_currentFile.string(), origin->m_rangeBegin.first, origin->m_rangeBegin.second, origin->m_rangeEnd.first, origin->m_rangeEnd.second, 0, false, 0)),
        ctx.m_currentFile,
        causer->m_rangeBegin,
        causer->m_rangeEnd
    );
}

SemanticError SemanticAnalyzer::cannotInferType(const AST::VariableDeclarationNode* causer, BrawContext& ctx) {
    return SemanticError(
        fmt::format("Cannot infer type of variable {}", Utils::getIdentifier(causer->m_name)),
        ctx.m_currentFile,
        causer->m_rangeBegin,
        causer->m_rangeEnd
    );
}

SemanticError SemanticAnalyzer::notInLoop(const AST::ContinueNode* causer, BrawContext& ctx) {
    return SemanticError(
        "Cannot use continue outside of loop",
        ctx.m_currentFile,
        causer->m_rangeBegin,
        causer->m_rangeEnd
    );
}

SemanticError SemanticAnalyzer::notInLoop(const AST::BreakNode* causer, BrawContext& ctx) {
    return SemanticError(
        "Cannot use break outside of loop or switch",
        ctx.m_currentFile,
        causer->m_rangeBegin,
        causer->m_rangeEnd
    );
}

bool SemanticAnalyzer::hasOperator(const TypeInfo& type, const std::string& operatorName) {
    static const std::unordered_set<std::string> operators = {
        "+", "-"
    };
    if(Rules::isPtr(type.m_name))
        return operators.contains(operatorName);

    return type.m_operators.contains(operatorName);
}

std::unordered_set<AST::Node::Type> SemanticAnalyzer::expressionWhitelist = {
    AST::Node::Literal, AST::Node::VariableAccess, AST::Node::FunctionCall,
    AST::Node::BinaryOperator, AST::Node::UnaryOperator
};