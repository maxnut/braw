#include "parser/parser.hpp"
#include "../unary_operator.hpp"
#include "../macro_parameter.hpp"
#include "rules.hpp"
#include <memory>

Result<std::shared_ptr<AST::Node>> Parser::parseOperand(TokenCursor& cursor, ParserContext& ctx) {
    if(Rules::isMacroCall(cursor))
        return parseMacroCall(cursor, ctx);
    else if(Rules::isMacroParameter(cursor))
        return parseMacroParameter(cursor, ctx);

    std::shared_ptr<AST::UnaryOperatorNode> unary = nullptr;
    std::pair<uint32_t, uint32_t> rangeBegin = {cursor.get().value().m_line, cursor.get().value().m_column};

    if(cursor.get().value().m_value == "&" || cursor.get().value().m_value == "*") {
        unary = std::make_shared<AST::UnaryOperatorNode>();
        unary->m_operator = cursor.get().value().m_value;
        unary->m_rangeBegin = rangeBegin;
        cursor.next();
        auto operandOpt = parseOperand(cursor, ctx);
        if(!operandOpt)
            return std::unexpected{operandOpt.error()};
        unary->m_operand = std::move(operandOpt.value());
        return unary;
    }
    
    auto primaryOpt = parsePrimary(cursor, ctx);
    if(!primaryOpt)
        return std::unexpected{primaryOpt.error()};

    std::shared_ptr<AST::Node> ret = std::move(primaryOpt.value());

    while(cursor.hasNext()) {
        if (cursor.get().value().m_type == Token::LEFT_BRACKET) {
            ret->m_rangeEnd = {cursor.get().value().m_line, cursor.get().value().m_column};
            cursor.next();
            auto indexOpt = parseExpression(cursor, ctx);
            if(!indexOpt)
                return std::unexpected{indexOpt.error()};
            if(!expectTokenType(cursor.get().value(), Token::RIGHT_BRACKET))
                return unexpectedTokenExpectedType(cursor.value(), Token::RIGHT_BRACKET, ctx.m_path);
            std::shared_ptr<AST::UnaryOperatorNode> subscript = std::make_shared<AST::UnaryOperatorNode>();
            subscript->m_rangeBegin = {cursor.get().value().m_line, cursor.get().value().m_column};
            subscript->m_operator = "[]";
            subscript->m_expression = std::move(indexOpt.value());
            subscript->m_operand = std::move(ret);
            ret = std::move(subscript);
            cursor.tryNext();
        }
        else if(cursor.get().value().m_value == "." || cursor.get().value().m_value == "->") {
            ret->m_rangeEnd = {cursor.get().value().m_line, cursor.get().value().m_column};
            auto dotArrowOpt = parseDotArrow(cursor, std::move(ret), ctx);
            if(!dotArrowOpt)
                return std::unexpected{dotArrowOpt.error()};
            ret = std::move(dotArrowOpt.value());
            continue;
        }
        break;
    }

    ret->m_rangeEnd = {cursor.get().value().m_line, cursor.get().value().m_column};
    return ret;
}