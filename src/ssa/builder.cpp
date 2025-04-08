#include "builder.hpp"
#include "parser/nodes/function_call.hpp"
#include "parser/nodes/literal.hpp"
#include "parser/nodes/unary_operator.hpp"
#include "parser/nodes/variable_access.hpp"
#include "rules.hpp"
#include "ssa/instruction.hpp"
#include "ssa/operand.hpp"
#include "ssa/operation.hpp"
#include "utils.hpp"
#include <memory>

namespace SSA {

template <typename T>
std::shared_ptr<T> cast(const std::shared_ptr<void>& ptr) {
    return std::static_pointer_cast<T>(ptr);
}

std::shared_ptr<Operation> operation(Operation::Type t, const TypeInfo& ti, std::shared_ptr<Operand> o1, std::shared_ptr<Operand> o2 = nullptr) { return std::make_shared<Operation>(t, ti, o1, o2); }

std::vector<File> Builder::build(AST::FileNode* root, BrawContext& context) {
    static std::unordered_set<std::filesystem::path> imported;
    std::vector<File> files;

    for(auto& imp : root->m_imports) {
        if(imported.contains(imp->m_path)) continue;
        imported.insert(imp->m_path);
        auto imports = build(imp.get(), context);
        for(auto& imp2 : imports) {
            if(imp2.m_functions.size() <= 0) continue;
            files.push_back(std::move(imp2));
        }
    }

    File file;
    file.m_path = root->m_path;

    file.m_functions.reserve(root->m_functions.size()); // avoid vector reallocation
    for(auto& func : root->m_functions) {
        file.m_functions.push_back(build(func.get(), context));
    }

    files.push_back(std::move(file));

    return files;
}

Function Builder::build(const AST::FunctionDefinitionNode* node, BrawContext& context) {
    Function f;
    f.m_external = node->m_signature.m_external;   

    FunctionContext ctx;
    ctx.m_function = &f;
    
    if(context.getTypeInfo(node->m_signature.m_returnType).value().m_size != 0) {
        if(context.getTypeInfo(node->m_signature.m_returnType).value().m_builtin) {
            if(node->m_signature.m_returnType.m_name == FLOAT_T || node->m_signature.m_returnType.m_name == DOUBLE_T)
                f.m_optReturn = makeOrGetRegister("%returnF", ctx);
            else
                f.m_optReturn = makeOrGetRegister("%return", ctx);
        }
        else {
            f.m_optReturn = makeOrGetRegister("%returnPtr", ctx);
            f.m_args.push_back(f.m_optReturn);
        }

        ctx.m_returnRegister = f.m_optReturn;
    }

    for(auto& arg : node->m_signature.m_parameters) {
        f.m_args.push_back(makeOrGetRegister("%" + arg->m_name.m_name + "_0", ctx));
        f.m_args.back()->m_typeInfo = context.getTypeInfo(arg->m_type).value();
    }

    f.m_name = node->m_signature.m_name;

    if(!node->m_signature.m_external) {
        ctx.m_instructions.push_back(std::make_shared<Label>(node->m_rangeBegin, node->m_signature.m_name));
        build(node->m_scope.get(), context, ctx);
        if(ctx.m_instructions.back()->m_type != Instruction::Return)
            ctx.m_instructions.push_back(std::make_shared<Instruction>(Instruction::Return, node->m_rangeEnd));
        f.m_instructions = std::move(ctx.m_instructions);
    }

    return f;
}

void Builder::build(AST::ScopeNode* node, BrawContext& context, FunctionContext& ictx) {
    ictx.m_scopeDepth++;
    for(auto& in : node->m_instructions)
        build(in.get(), context, ictx);
    ictx.m_scopeDepth--;
}

void Builder::build(AST::Node* node, BrawContext& context, FunctionContext& ictx) {
    switch(node->m_type) {
        case AST::Node::VariableDeclaration:
            return build((AST::VariableDeclarationNode*)node, context, ictx);
        case AST::Node::If:
            return build((AST::IfNode*)node, context, ictx);
        case AST::Node::While:
            return build((AST::WhileNode*)node, context, ictx);
        case AST::Node::For:
            return build((AST::ForNode*)node, context, ictx);
        case AST::Node::Return:
            return build((AST::ReturnNode*)node, context, ictx);
        case AST::Node::Type::BinaryOperator:
            if(static_cast<const AST::BinaryOperatorNode*>(node)->m_operator == "=")
                return buildAssignment(static_cast<AST::BinaryOperatorNode*>(node), context, ictx);
        case AST::Node::Scope:
            return build((AST::ScopeNode*)node, context, ictx);
        default:
            buildExpression(node, context, ictx);
            break;
    }
}

void Builder::build(AST::VariableDeclarationNode* node, BrawContext& context, FunctionContext& ictx) {
    auto reg = makeOrGetRegister("%" + node->m_name.m_name + "_" + std::to_string(ictx.m_scopeDepth), ictx);
    reg->m_typeInfo = context.getTypeInfo(node->m_type).value();
    reg->m_scale = node->m_scale;
    reg->m_typeInfo.m_builtin = reg->m_scale <= 1 && reg->m_typeInfo.m_builtin;

    auto label = std::make_shared<Label>(node->m_rangeEnd, "." + std::to_string((uintptr_t)node));

    if(node->m_retain)
        ictx.m_function->m_retains.insert({reg->m_id, reg});
    else if(!reg->m_typeInfo.m_builtin)
        ictx.m_instructions.push_back(std::make_shared<Allocate>(node->m_rangeBegin, reg, reg->m_typeInfo.m_size));

    if(node->m_value) {
        if(node->m_retain) {
            auto guard = makeOrGetRegister("%" + node->m_name.m_name + "_" + std::to_string(ictx.m_scopeDepth) + "_guard", ictx);
            guard->m_typeInfo = context.getTypeInfo(BOOL_T).value();
            guard->m_scale = 1;
            guard->m_typeInfo.m_builtin = true;
            ictx.m_function->m_retains.insert({guard->m_id, guard});
            ictx.m_instructions.push_back(
                std::make_shared<Jump>(Instruction::JumpTrue, node->m_rangeBegin, label, guard)
            );
            auto v = std::make_shared<Immediate>();
            v->m_value = true;
            assign(v, load(v), node->m_rangeBegin, ictx);
        }
        auto op = buildExpression(node->m_value.get(), context, ictx);
        if(op->m_type == Operand::Type::Immediate && std::holds_alternative<std::string>(std::static_pointer_cast<Immediate>(op)->m_value)) {
            assign(reg, point(op), node->m_rangeBegin, ictx);
            return;
        }
        assign(reg, load(op), node->m_rangeBegin, ictx);
        if(node->m_retain)
            ictx.m_instructions.push_back(label);
    }
}

void Builder::build(AST::WhileNode* node, BrawContext& context, FunctionContext& ictx) {
    auto label = std::make_shared<Label>(node->m_condition->m_rangeBegin, "." + std::to_string((uintptr_t)node) + "_condition");
    auto labelBody = std::make_shared<Label>(node->m_then->m_rangeBegin, "." + std::to_string((uintptr_t)node) + "_body");
    if(!node->m_do)
        ictx.m_instructions.push_back(std::make_shared<Jump>(Instruction::Jump, node->m_rangeBegin, label));
    ictx.m_instructions.push_back(label);
    build(node->m_then.get(), context, ictx);
    ictx.m_instructions.push_back(label);
    auto condition = buildExpression(node->m_condition.get(), context, ictx);
    ictx.m_instructions.push_back(
        std::make_shared<Jump>(Instruction::JumpTrue, node->m_rangeEnd, labelBody, condition)
    );
}

void Builder::build(AST::ForNode* node, BrawContext& context, FunctionContext& ictx) {
    build(node->m_initializer.get(), context, ictx);
    auto labelCondition = std::make_shared<Label>(node->m_condition->m_rangeBegin, "." + std::to_string((uintptr_t)node) + "_condition");
    auto labelBody = std::make_shared<Label>(node->m_body->m_rangeBegin, "." + std::to_string((uintptr_t)node) + "_body");
    ictx.m_instructions.push_back(
        std::make_shared<Jump>(Instruction::Jump, node->m_rangeBegin, labelCondition)
    );
    ictx.m_instructions.push_back(labelBody);
    build(node->m_body.get(), context, ictx);
    build(node->m_increment.get(), context, ictx);
    ictx.m_instructions.push_back(labelCondition);
    auto condition = buildExpression(node->m_condition.get(), context, ictx);
    ictx.m_instructions.push_back(
        std::make_shared<Jump>(Instruction::JumpTrue, node->m_body->m_rangeEnd, labelBody, condition)
    );
}


void Builder::build(AST::IfNode* node, BrawContext& context, FunctionContext& ictx) {
    auto condition = buildExpression(node->m_condition.get(), context, ictx);
    auto label = std::make_shared<Label>(node->m_rangeBegin, "." + std::to_string((uintptr_t)node));
    ictx.m_instructions.push_back(
        std::make_shared<Jump>(Instruction::JumpFalse, node->m_condition->m_rangeBegin, label, condition)
    );
    build(node->m_then.get(), context, ictx);
    if(node->m_else) {
        auto elseLabel = std::make_shared<Label>(node->m_else->m_rangeBegin, "." + std::to_string((uintptr_t)node) + "_else");
        ictx.m_instructions.push_back(
            std::make_shared<Jump>(Instruction::Jump, node->m_condition->m_rangeBegin, label)
        );
        ictx.m_instructions.push_back(label);
        build(node->m_else.get(), context, ictx);
        ictx.m_instructions.push_back(elseLabel);
        return;
    }
    ictx.m_instructions.push_back(label);
}

void Builder::build(AST::ReturnNode* node, BrawContext& context, FunctionContext& ictx) {
    if(node->m_value) {
        auto op = buildExpression(node->m_value.get(), context, ictx);
        if(op->m_type == Operand::Type::Immediate && std::holds_alternative<std::string>(std::static_pointer_cast<Immediate>(op)->m_value))
            assign(ictx.m_returnRegister, point(op), node->m_rangeBegin, ictx);
        else
            assign(ictx.m_returnRegister, load(op), node->m_rangeBegin, ictx);
    }

    ictx.m_instructions.push_back(std::make_shared<Instruction>(Instruction::Return, node->m_rangeBegin));
}

void Builder::buildAssignment(AST::BinaryOperatorNode* node, BrawContext& context, FunctionContext& ictx) {
    auto right = buildExpression(node->m_right.get(), context, ictx);
    auto left = buildExpression(node->m_left.get(), context, ictx);

    if(ictx.m_instructions.back()->m_type == Instruction::Assign && std::static_pointer_cast<Assignment>(ictx.m_instructions.back())->m_operation->m_type == Operation::Dereference) {
        std::static_pointer_cast<Assignment>(ictx.m_instructions.back())->m_operation->m_type = Operation::PartialDereference;
        if(left->m_type != Operand::Type::Address) {
            TypeInfo old = left->m_typeInfo;
            left = std::make_shared<Address>(left->m_typeInfo, cast<Register>(left));
        }
    }

    if(right->m_type == Operand::Type::Immediate && std::holds_alternative<std::string>(std::static_pointer_cast<Immediate>(right)->m_value)) {
        assign(left, point(right), node->m_rangeBegin, ictx);
        return;
    }

    assign(left, load(right), node->m_rangeBegin, ictx);
}

std::shared_ptr<Operand> Builder::buildExpression(AST::Node* node, BrawContext& context, FunctionContext& ictx) {
    switch(node->m_type) {
        case AST::Node::BinaryOperator:
            return buildBinaryOperator(static_cast<AST::BinaryOperatorNode*>(node), context, ictx);
        case AST::Node::UnaryOperator:
            return buildUnaryOperator(static_cast<AST::UnaryOperatorNode*>(node), context, ictx);
        case AST::Node::Literal: {
            std::array<TypeInfo, 7> types = {
                context.getTypeInfo(INT_T).value(), context.getTypeInfo(LONG_T).value(), context.getTypeInfo(FLOAT_T).value(), context.getTypeInfo(DOUBLE_T).value(),
                context.getTypeInfo(BOOL_T).value(), Utils::makePointer(context.getTypeInfo(CHAR_T).value()), Utils::makePointer(context.getTypeInfo(VOID_T).value())
            };
            auto lit = static_cast<AST::LiteralNode*>(node);
            return std::make_shared<Immediate>(lit->m_value, types[lit->m_value.index()]);
        }
        case AST::Node::VariableAccess:{
            auto var = static_cast<AST::VariableAccessNode*>(node);
            for(int i = ictx.m_scopeDepth; i >= 0; i--) {
                if(ictx.m_registers.contains("%" + var->m_name.m_name + "_" + std::to_string(i)))
                    return ictx.m_registers["%" + var->m_name.m_name + "_" + std::to_string(i)];
            }
            break;
        }
        case AST::Node::FunctionCall:
            return buildCall(static_cast<AST::FunctionCallNode*>(node), context, ictx);
        default:
            break;
    }

    return {};
}


std::shared_ptr<Operand> Builder::buildBinaryOperator(AST::BinaryOperatorNode* node, BrawContext& context, FunctionContext& ictx) {
    if(node->m_operator == "||") {
        std::string name = "%" + std::to_string((uintptr_t)node);
        std::shared_ptr<Register> target = makeOrGetRegister(name, ictx);
        auto label = std::make_shared<Label>(node->m_right->m_rangeEnd, "." + std::to_string((uintptr_t)node));
        auto left = buildExpression(node->m_left.get(), context, ictx);
        assign(target, load(left), node->m_left->m_rangeEnd, ictx);
        ictx.m_instructions.push_back(std::make_shared<Jump>(Instruction::JumpTrue, node->m_right->m_rangeEnd, label, target));
        auto right = buildExpression(node->m_right.get(), context, ictx);
        auto orOp = std::make_shared<Operation>(Operation::Or, right->m_typeInfo, target, right);
        assign(target, orOp, node->m_right->m_rangeEnd, ictx);
        ictx.m_instructions.push_back(label);
        return target;
    }

    auto left = buildExpression(node->m_left.get(), context, ictx);
    auto right = buildExpression(node->m_right.get(), context, ictx);

    std::string name = "%" + std::to_string((uintptr_t)node);
    std::shared_ptr<Register> target = makeOrGetRegister(name, ictx);

    if(node->m_operator == "+")
        assign(target, operation(Operation::Add, left->m_typeInfo, left, right), node->m_rangeBegin, ictx);
    else if(node->m_operator == "*")
        assign(target, operation(Operation::Multiply, left->m_typeInfo, left, right), node->m_rangeBegin, ictx);
    else if(node->m_operator == "/")
        assign(target, operation(Operation::Divide, left->m_typeInfo, left, right), node->m_rangeBegin, ictx);
    else if(node->m_operator == "%")
        assign(target, operation(Operation::Modulo, left->m_typeInfo, left, right), node->m_rangeBegin, ictx);
    else if(node->m_operator == "-")
        assign(target, operation(Operation::Subtract, left->m_typeInfo, left, right), node->m_rangeBegin, ictx);
    else if(node->m_operator == "==")
        assign(target, operation(Operation::CompareEquals, context.getTypeInfo(left->m_typeInfo.m_operators.at("==").m_returnType).value(), left, right), node->m_rangeBegin, ictx);
    else if(node->m_operator == "!=")
        assign(target, operation(Operation::CompareNotEquals, context.getTypeInfo(left->m_typeInfo.m_operators.at("!=").m_returnType).value(), left, right), node->m_rangeBegin, ictx);
    else if(node->m_operator == ">")
        assign(target, operation(Operation::CompareGreater, context.getTypeInfo(left->m_typeInfo.m_operators.at(">").m_returnType).value(), left, right), node->m_rangeBegin, ictx);
    else if(node->m_operator == "<")
        assign(target, operation(Operation::CompareLess, context.getTypeInfo(left->m_typeInfo.m_operators.at("<").m_returnType).value(), left, right), node->m_rangeBegin, ictx);
    else if(node->m_operator == "<=")
        assign(target, operation(Operation::CompareLessEquals, context.getTypeInfo(left->m_typeInfo.m_operators.at("<=").m_returnType).value(), left, right), node->m_rangeBegin, ictx);
    else if(node->m_operator == ">=")
        assign(target, operation(Operation::CompareGreaterEquals, context.getTypeInfo(left->m_typeInfo.m_operators.at(">=").m_returnType).value(), left, right), node->m_rangeBegin, ictx);
    else if(node->m_operator == "&" || node->m_operator == "&&")
        assign(target, operation(Operation::And, left->m_typeInfo, left, right), node->m_rangeBegin, ictx);
    else if(node->m_operator == "|")
        assign(target, operation(Operation::Or, left->m_typeInfo, left, right), node->m_rangeBegin, ictx);
    else if(node->m_operator == "^")
        assign(target, operation(Operation::Xor, left->m_typeInfo, left, right), node->m_rangeBegin, ictx);
    
    return target;
}

std::shared_ptr<Operand> Builder::dotOperator(AST::UnaryOperatorNode* node, std::shared_ptr<Operand> op, BrawContext& context, FunctionContext& ictx) {
    MemberInfo member = op->m_typeInfo.m_members.at(node->m_data);
    int64_t offset = member.m_offset; 
    auto tMember = context.getTypeInfo(member.m_type).value();
    switch(op->m_type) {
        default:
            return std::make_shared<Address>(tMember, cast<Register>(op), offset, nullptr, member.m_scale);
        case Operand::Address: {
            auto addr = cast<Address>(op);
            return std::make_shared<Address>(tMember, addr->m_base, addr->m_offset + offset, nullptr, member.m_scale);
        }
    }
}

std::shared_ptr<Operand> Builder::dereferenceOperator(AST::UnaryOperatorNode* node, std::shared_ptr<Operand> op, BrawContext& context, FunctionContext& ictx) {
    auto tmp = op;
    auto ret = makeOrGetRegister("%" + std::to_string((uintptr_t)node), ictx);
    ret->m_typeInfo = Utils::getRawType(op->m_typeInfo, context).value();
    assign(ret, operation(Operation::Dereference, ret->m_typeInfo, op), node->m_rangeBegin, ictx);
    return ret;
}

std::shared_ptr<Operand> Builder::addressOperator(AST::UnaryOperatorNode* node, std::shared_ptr<Operand> op, BrawContext& context, FunctionContext& ictx) {
    auto tmp = op;
    auto ret = makeOrGetRegister("%" + std::to_string((uintptr_t)node), ictx);
    ret->m_typeInfo = Utils::makePointer(op->m_typeInfo);
    assign(ret, point(op), node->m_rangeBegin, ictx);
    return ret;
}

std::shared_ptr<Operand> Builder::subscriptOperator(AST::UnaryOperatorNode* node, std::shared_ptr<Operand> op, BrawContext& context, FunctionContext& ictx) {
    auto index = buildExpression(node->m_expression.get(), context, ictx);
    if(index->m_type != Operand::Register) {
        auto tmp = makeOrGetRegister("%" + std::to_string((uintptr_t)node) + "_0", ictx);
        tmp->m_typeInfo = op->m_typeInfo;
        assign(tmp, load(index), node->m_rangeBegin, ictx);
        index = tmp;
    }
    if(op->m_type != Operand::Register) {
        auto tmp = makeOrGetRegister("%" + std::to_string((uintptr_t)node) + "_1", ictx);
        tmp->m_typeInfo = op->m_typeInfo;
        if(op->m_type == Operand::Address && cast<Address>(op)->m_scaleSize > 1)
            assign(tmp, point(op), node->m_rangeBegin, ictx);
        else
            assign(tmp, load(op), node->m_rangeBegin, ictx);
        op = tmp;
    }

    if(op->m_typeInfo.m_name == INT_T) {
        auto tmp = makeOrGetRegister(cast<Register>(index)->m_id + "_0", ictx);
        assign(tmp, operation(Operation::Upsize, context.getTypeInfo(LONG_T).value(), index), node->m_rangeBegin, ictx);
        index = tmp;
    }
    
    TypeInfo raw = Utils::getRawType(op->m_typeInfo, context).value();
    auto addr = std::make_shared<Address>(raw, cast<Register>(op), 0, cast<Register>(index), 0);
    addr->m_scale = raw.m_size;
    return addr;
}

std::shared_ptr<Operand> Builder::castOperator(AST::UnaryOperatorNode* node, std::shared_ptr<Operand> op, BrawContext& context, FunctionContext& ictx) {
    std::shared_ptr<Operand> ret;
    if(Rules::isPtr(node->m_data) || node->m_data.m_name == LONG_T) {
        if(Rules::isPtr(op->m_typeInfo.m_name) || op->m_typeInfo.m_name == LONG_T)
            ret = op;
        else if(op->m_typeInfo.m_name == INT_T || op->m_typeInfo.m_name == CHAR_T) {
            if(op->m_type == Operand::Immediate) {
                auto imm = cast<Immediate>(op);
                if(imm->m_typeInfo.m_name == INT_T)
                    imm->m_value = (long)std::get<int>(imm->m_value);
                else
                    imm->m_value = (long)std::get<char>(imm->m_value);
                ret = imm;
            }
            else {
                ret = makeOrGetRegister("%" + std::to_string((uintptr_t)node), ictx);
                assign(ret, operation(Operation::Upsize, context.getTypeInfo(node->m_data).value(), op), node->m_rangeBegin, ictx);
            }
        }
    }
    else if(node->m_data.m_name == INT_T) {
        if(op->m_typeInfo.m_name == CHAR_T) {
            if(op->m_type == Operand::Immediate) {
                auto imm = cast<Immediate>(op);
                imm->m_value = (int)std::get<char>(imm->m_value);
                ret = imm;
            }
            else {
                ret = makeOrGetRegister("%" + std::to_string((uintptr_t)node), ictx);
                assign(ret, operation(Operation::Upsize, context.getTypeInfo(node->m_data).value(), op), node->m_rangeBegin, ictx);
            }
        }
        else if(op->m_typeInfo.m_name == LONG_T) {
            if(op->m_type == Operand::Immediate) {
                auto imm = cast<Immediate>(op);
                imm->m_value = (int)std::get<long>(imm->m_value);
                ret = imm;
            }
            else {
                ret = makeOrGetRegister("%" + std::to_string((uintptr_t)node), ictx);
                assign(ret, operation(Operation::Downsize, context.getTypeInfo(node->m_data).value(), op), node->m_rangeBegin, ictx);
            }
        }
    }
    else if(node->m_data.m_name == CHAR_T) {
        if(op->m_typeInfo.m_name == INT_T) {
            if(op->m_type == Operand::Immediate) {
                auto imm = cast<Immediate>(op);
                imm->m_value = (char)std::get<int>(imm->m_value);
                ret = imm;
            }
            else {
                ret = makeOrGetRegister("%" + std::to_string((uintptr_t)node), ictx);
                assign(ret, operation(Operation::Downsize, context.getTypeInfo(node->m_data).value(), op), node->m_rangeBegin, ictx);
            }
        }
        else if(op->m_typeInfo.m_name == LONG_T) {
            if(op->m_type == Operand::Immediate) {
                auto imm = cast<Immediate>(op);
                imm->m_value = (char)std::get<long>(imm->m_value);
                ret = imm;
            }
            else {
                ret = makeOrGetRegister("%" + std::to_string((uintptr_t)node), ictx);
                assign(ret, operation(Operation::Downsize, context.getTypeInfo(node->m_data).value(), op), node->m_rangeBegin, ictx);
            }
        }
    }
    if(ret->m_type == Operand::Register) {
        std::shared_ptr<Register> reg = cast<Register>(ret);
        std::shared_ptr<Register> clone = std::make_shared<Register>(reg->m_id, context.getTypeInfo(node->m_data).value());
        clone->m_scale = reg->m_scale;
        ret = clone;
    }
    else if(ret->m_type == Operand::Address) {
        std::shared_ptr<Address> addr = cast<Address>(ret);
        std::shared_ptr<Address> clone = std::make_shared<Address>(context.getTypeInfo(node->m_data).value(), addr->m_base, addr->m_offset, addr->m_index, addr->m_scaleSize);
        clone->m_scale = addr->m_scale;
        ret = clone;
    }
    return ret;
}

std::shared_ptr<Operand> Builder::logicalNotOperator(AST::UnaryOperatorNode* node, std::shared_ptr<Operand> op, BrawContext& context, FunctionContext& ictx) {
    auto ret = makeOrGetRegister("%" + std::to_string((uintptr_t)node), ictx);
    assign(ret, operation(Operation::LogicalNot, context.getTypeInfo(BOOL_T).value(), op), node->m_rangeBegin, ictx);
    return ret;
}

std::shared_ptr<Operand> Builder::buildUnaryOperator(AST::UnaryOperatorNode* node, BrawContext& context, FunctionContext& ictx) {
    std::shared_ptr<Operand> ret;
    std::shared_ptr<Operand> op = buildExpression(node->m_operand.get(), context, ictx);

    if(node->m_operator == ".") {
        ret = dotOperator(node, op, context, ictx);
    }
    else if(node->m_operator == "&") 
        ret = addressOperator(node, op, context, ictx);
    else if(node->m_operator == "*") 
        ret = dereferenceOperator(node, op, context, ictx);
    else if(node->m_operator == "->") {
        ret = dereferenceOperator(node, op, context, ictx);
        ret = dotOperator(node, ret, context, ictx);
    }
    else if(node->m_operator == "[]") {
        ret = subscriptOperator(node, op, context, ictx);
    }
    else if(node->m_operator == "cast") {
        ret = castOperator(node, op, context, ictx);
    }
    else if(node->m_operator == "!") {
        ret = logicalNotOperator(node, op, context, ictx);
    }

    return ret;
}

std::shared_ptr<Operand> Builder::buildCall(AST::FunctionCallNode* node, BrawContext& context, FunctionContext& ictx) {
    auto call = std::make_shared<Call>(node->m_rangeBegin);
    call->m_id = node->m_name;

    std::vector<TypeInfo> tmpTypes;

    for(auto& param : node->m_parameters) {
        auto op = buildExpression(param.get(), context, ictx);

        TypeInfo t = op->m_typeInfo;

        if(param->m_type == AST::Node::UnaryOperator && static_cast<const AST::UnaryOperatorNode*>(param.get())->m_operator == "cast")
            t = context.getTypeInfo(static_cast<const AST::UnaryOperatorNode*>(param.get())->m_data).value();
        call->m_parameters.push_back(op);
        tmpTypes.push_back(t);
    }

    std::string name = "%" + std::to_string((uintptr_t)node);

    auto fun = context.getFunction(node->m_name, tmpTypes);
    call->m_returnType = fun->m_returnType;
    if(fun->m_returnType.m_size != 0) {
        call->m_optReturn = makeOrGetRegister(name, ictx);
        call->m_optReturn->m_typeInfo = fun->m_returnType;
    }

    ictx.m_instructions.push_back(call);
    return makeOrGetRegister(name, ictx);
}


void Builder::assign(std::shared_ptr<Operand> to, std::shared_ptr<Operation> operation, std::pair<uint32_t, uint32_t> range, FunctionContext& ctx) {
    if(to->m_typeInfo.m_name == "")
        to->m_typeInfo = operation->m_typeInfo;
    auto instr = std::make_shared<Assignment>(range);
    instr->m_to = to;
    instr->m_operation = operation;
    ctx.m_instructions.push_back(instr);
}

std::shared_ptr<Register> Builder::makeOrGetRegister(const std::string& name, FunctionContext& ctx) {
    if(ctx.m_registers.contains(name))
        return ctx.m_registers[name];

    std::shared_ptr<Register> reg = std::make_shared<Register>(name);
    ctx.m_registers[name] = reg;
    return reg;
}

std::shared_ptr<Operation> Builder::load(std::shared_ptr<Operand> op) {
    return operation(Operation::Load, op->m_typeInfo, op);
}

std::shared_ptr<Operation> Builder::point(std::shared_ptr<Operand> op) {
    return operation(Operation::Point, Utils::makePointer(op->m_typeInfo), op);
}

}