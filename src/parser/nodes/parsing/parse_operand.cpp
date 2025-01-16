#include "parser/parser.hpp"
#include "../unary_operator.hpp"

Result<std::unique_ptr<AST::Node>> Parser::parseOperand(TokenCursor& cursor) {
    std::unique_ptr<AST::UnaryOperatorNode> unary = nullptr;

    if(cursor.get().value().m_value == "&" || cursor.get().value().m_value == "*") {
        unary = std::make_unique<AST::UnaryOperatorNode>();
        unary->m_operator = cursor.get().value().m_value;
        cursor.next();
    }
    
    auto primaryOpt = parsePrimary(cursor);
    if(!primaryOpt)
        return std::unexpected{primaryOpt.error()};

    std::unique_ptr<AST::Node> ret = std::move(primaryOpt.value());

    if(unary) {
        unary->m_operand = std::move(ret);
        ret = std::move(unary);
    }

    while(cursor.hasNext()) {
        if(cursor.get().value().m_value == "." || cursor.get().value().m_value == "->") {
            auto dotArrowOpt = parseDotArrow(cursor, std::move(ret));
            if(!dotArrowOpt)
                return std::unexpected{dotArrowOpt.error()};
            ret = std::move(dotArrowOpt.value());
            continue;
        }
        break;
    }

    return ret;
}