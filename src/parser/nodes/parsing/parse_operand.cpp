#include "parser/parser.hpp"
#include "parser/nodes/address.hpp"

std::unique_ptr<EvaluatableNode> Parser::parseOperand(std::shared_ptr<FileNode> file, TokenCursor& cursor, ParserFunctionContext& ctx) {
    std::unique_ptr<AddressNode> address = nullptr;

    if(cursor.get().value().m_value == "&") {
        address = std::make_unique<AddressNode>();
        cursor.next();
    }
    
    std::unique_ptr<EvaluatableNode> ret = parsePrimary(file, cursor, ctx);

    if(address) {
        if(ret->m_memoryType == ValueType::PRVALUE || ret->m_memoryType == ValueType::XVALUE) {
            m_message.unexpectedValueCategories(ret->m_memoryType, {ValueType::LVALUE});
            return nullptr;
        }

        address->m_base = std::move(ret);
        address->m_size = 8;
        address->m_memoryType = ValueType::PRVALUE;
        address->m_type = makePointer(address->m_base->m_type);
        ret = std::move(address);
    }

    return ret;
}