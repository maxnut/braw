#include "../identifier.hpp"
#include "parser/nodes/binary_operator.hpp"
#include "parser/nodes/for.hpp"
#include "parser/nodes/function_call.hpp"
#include "parser/nodes/if.hpp"
#include "parser/nodes/literal.hpp"
#include "parser/nodes/macro_call.hpp"
#include "parser/nodes/macro_parameter.hpp"
#include "parser/nodes/macro_parameter_reference.hpp"
#include "parser/nodes/return.hpp"
#include "parser/nodes/scope.hpp"
#include "parser/nodes/struct.hpp"
#include "parser/nodes/unary_operator.hpp"
#include "parser/nodes/variable_access.hpp"
#include "parser/nodes/variable_declaration.hpp"
#include "parser/nodes/while.hpp"
#include "parser/parser.hpp"
#include <memory>

Result<std::shared_ptr<AST::MacroParameterNode>> Parser::parseMacroDotChain(TokenCursor& cursor, std::shared_ptr<AST::MacroParameterNode> left, ParserContext& ctx) {
    std::shared_ptr<AST::MacroParameterDotNode> macroDot = std::make_shared<AST::MacroParameterDotNode>();
    macroDot->m_rangeBegin = {cursor.get().value().m_line, cursor.get().value().m_column};
    macroDot->m_prev = left;
    cursor.next();
    macroDot->m_value = cursor.get().value().m_value;
    macroDot->m_rangeEnd = {cursor.get().value().m_line, cursor.get().value().m_column};

    if(cursor.hasNext() && cursor.next().get().value().m_value == "=>")
        return parseMacroDotChain(cursor, macroDot, ctx);
    
    return macroDot;
}

Result<std::shared_ptr<AST::MacroParameterNode>> Parser::parseMacroParameter(TokenCursor& cursor, ParserContext& ctx) {
    std::shared_ptr<AST::MacroParameterNode> ret = nullptr;
    if(cursor.get().value().m_value == "#" && cursor.next().get().prev().value().m_type == Token::IDENTIFIER && cursor.next(2).get().prev(2).value().m_type == Token::LEFT_PAREN) {
        auto param = std::make_shared<AST::MacroParameterFunctionNode>();
        param->m_rangeBegin = {cursor.get().value().m_line, cursor.get().value().m_column};
        param->m_name = cursor.next().get().value().m_value;
        if(!expectTokenType(cursor.next().get().value(), Token::RIGHT_PAREN))
            return unexpectedTokenExpectedType(cursor.value(), Token::RIGHT_PAREN, ctx.m_path);
        param->m_rangeEnd = {cursor.get().value().m_line, cursor.get().value().m_column};
        cursor.tryNext();
        ret = param;
    }
    else if(cursor.get().value().m_value == "#" && cursor.next().get().prev().value().m_type == Token::QUOTE) {
        auto param = std::make_shared<AST::MacroParameterValueNode>();
        param->m_rangeBegin = {cursor.get().value().m_line, cursor.get().value().m_column};
        cursor.next(2);
        param->m_value = cursor.get().value().m_value;
        if(!expectTokenType(cursor.next().get().value(), Token::QUOTE))
            return unexpectedTokenExpectedType(cursor.value(), Token::QUOTE, ctx.m_path);
        param->m_rangeEnd = {cursor.get().value().m_line, cursor.get().value().m_column};
        cursor.tryNext();
        ret = param;
    }
    else if(cursor.get().value().m_value == "#" && cursor.next().get().prev().value().m_value == "<") {
        auto param = std::make_shared<AST::MacroParameterTypeNode>();
        param->m_rangeBegin = {cursor.get().value().m_line, cursor.get().value().m_column};
        cursor.next(2);
        param->m_type = cursor.get().value().m_value;
        if(!expectTokenValue(cursor.next().get().value(), ">"))
            return unexpectedTokenExpectedValue(cursor.value(), ">", ctx.m_path);
        param->m_rangeEnd = {cursor.get().value().m_line, cursor.get().value().m_column};
        cursor.tryNext();
        ret = param;
    }
    else if(cursor.get().value().m_value == "#") {
        auto param = std::make_shared<AST::MacroParameterASTNode>();               
        param->m_rangeBegin = {cursor.get().value().m_line, cursor.get().value().m_column};
        auto inOpt = parseMacroParameterReference(cursor, ctx);
        if(!inOpt)
            return std::unexpected{inOpt.error()};
        std::shared_ptr<AST::MacroParameterReferenceNode> ref = inOpt.value();
        param->m_ast = ref;
        param->m_rangeEnd = {cursor.get().value().m_line, cursor.get().value().m_column};
        ret = param;
    }
    else {
        auto param = std::make_shared<AST::MacroParameterASTNode>();               
        param->m_rangeBegin = {cursor.get().value().m_line, cursor.get().value().m_column};
        auto inOpt = parseInstruction(cursor, ctx);
        if(!inOpt)
            return std::unexpected{inOpt.error()};
        param->m_ast = inOpt.value();
        param->m_rangeEnd = {cursor.get().value().m_line, cursor.get().value().m_column};
        ret = param;
    }

    if(cursor.get().value().m_value == "=>") {
        auto paramDotOpt = parseMacroDotChain(cursor, ret, ctx);
        if(!paramDotOpt)
            return std::unexpected{paramDotOpt.error()};
        ret = paramDotOpt.value();
        ret->m_parameterType = AST::MacroParameterType::Dot;
    }
    return ret;
}

Result<std::shared_ptr<AST::MacroCallNode>> Parser::parseMacroCall(TokenCursor& cursor, ParserContext& ctx) {
    std::shared_ptr<AST::MacroCallNode> call = std::make_shared<AST::MacroCallNode>();
    call->m_rangeBegin = {cursor.get().value().m_line, cursor.get().value().m_column};
    cursor.tryNext();
    auto identifierOpt = parseIdentifier(cursor, ctx);
    if(!identifierOpt) return std::unexpected{identifierOpt.error()};
    call->m_name = identifierOpt.value();

    if(cursor.get().value().m_type == Token::LEFT_PAREN) {
        cursor.tryNext();
        while(cursor.hasNext()) {
            auto paramOpt = parseMacroParameter(cursor, ctx);
            if(!paramOpt)
                return std::unexpected{paramOpt.error()};
            call->m_parameters.push_back(std::move(paramOpt.value()));
            if(cursor.get().value().m_type == Token::RIGHT_PAREN)
                break;
            if(!expectTokenType(cursor.get().next().value(), Token::COMMA))
                return unexpectedTokenExpectedType(cursor.value(), Token::COMMA, ctx.m_path);
        }
        cursor.tryNext();
    }
    
    call->m_rangeEnd = {cursor.get().value().m_line, cursor.get().value().m_column};
    return call;
}