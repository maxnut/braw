#include "ir/address.hpp"
#include "ir/operand.hpp"
#include "ir/register.hpp"
#include "ir_builder.hpp"
#include "parser/nodes/unary_operator.hpp"

#include <memory>
#include <string>

Operand IRBuilder::buildUnaryOperator(const AST::UnaryOperatorNode* node, BrawContext& context, IRFunctionContext& ictx) {
    Operand ret;
    Operand op = buildExpression(node->m_operand.get(), context, ictx);

    if(node->m_operator == ".") {
        auto t = getOperandType(op, context, ictx);
        int64_t offset = t.m_members.at(node->m_data).m_offset; 
        switch(op.index()) {
            default:
                ret = Address(std::get<std::shared_ptr<Register>>(op), offset);
                break;
            case 3: {
                Address addr = std::get<Address>(op);
                ret = Address(addr.m_base, addr.m_offset + offset);
                break;
            }
        }
    }
    else if(node->m_operator == "&") 
        ret = op;
    else if(node->m_operator == "*")
        ret = Address(std::get<std::shared_ptr<Register>>(op), 0);

    return ret;
}