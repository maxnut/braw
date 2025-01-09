#include "parser/parser.hpp"
#include "parser/nodes/address.hpp"
#include "parser/nodes/dereference.hpp"

std::unique_ptr<EvaluatableNode> Parser::parseOperand(std::shared_ptr<FileNode> file, TokenCursor& cursor, ParserFunctionContext& ctx) {
    std::unique_ptr<AddressNode> address = nullptr;
    std::unique_ptr<DereferenceNode> dereference = nullptr;

    if(cursor.get().value().m_value == "&") {
        address = std::make_unique<AddressNode>();
        cursor.next();
    }
    else if(cursor.get().value().m_value == "*") {
        dereference = std::make_unique<DereferenceNode>();
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
    else if(dereference) {
        if(!Rules::isPtr(ret->m_type.m_name)) {
            m_message.mismatchedTypes("pointer", ret->m_type.m_name);
            return nullptr;
        }

        dereference->m_base = std::move(ret);
        dereference->m_memoryType = ValueType::LVALUE;
        auto rawOpt = getRawType(dereference->m_base->m_type, file);

        if(!rawOpt) {
            m_message.unknownType(dereference->m_base->m_type.m_name);
            return nullptr;
        }
        
        dereference->m_type = rawOpt.value();
        dereference->m_size = dereference->m_type.m_size;
        ret = std::move(dereference);
    }

    return ret;
}