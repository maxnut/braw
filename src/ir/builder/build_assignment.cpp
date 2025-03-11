#include "ir/address.hpp"
#include "ir/instructions/basic.hpp"
#include "ir/register.hpp"
#include "ir_builder.hpp"
#include "parser/nodes/binary_operator.hpp"
#include "rules.hpp"
#include "type_info.hpp"
#include <memory>
#include <variant>

void IRBuilder::buildAssignment(const AST::BinaryOperatorNode* node, BrawContext& context, IRFunctionContext& ictx) {
    Operand left = buildExpression(node->m_left.get(), context, ictx);

    if(ictx.m_instructions.back()->m_type == Instruction::Dereference) {
        ictx.m_instructions.back()->m_type = Instruction::PartialDereference;
        if(!std::holds_alternative<Address>(left))
            left = Address(std::get<1>(left), 0, std::get<1>(left)->m_type);
    }
    Operand right = buildExpression(node->m_right.get(), context, ictx);

    if(left.index() == 1)
        moveToRegister(std::get<std::shared_ptr<Register>>(left)->m_id, right, context, ictx);
    else {
        Address ad = std::get<Address>(left);
        ictx.m_instructions.push_back(std::make_unique<BasicInstruction>(ad.m_typeInfo.m_builtin ? Instruction::Move : Instruction::Copy, left, right));
    }
}