#include "builder.hpp"
#include "ir/instruction.hpp"
#include "parser/nodes/function_call.hpp"
#include "parser/nodes/literal.hpp"
#include "parser/nodes/unary_operator.hpp"
#include "parser/nodes/variable_access.hpp"
#include "rules.hpp"
#include "ssa/copy_propagator.hpp"
#include "ssa/cse.hpp"
#include "utils.hpp"
#include <iostream>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace SSA {

template <typename T>
std::shared_ptr<T> cast(const std::shared_ptr<void>& ptr) {
    return std::static_pointer_cast<T>(ptr);
}

int64_t getJumpTarget(const std::string label, const std::vector<std::shared_ptr<Instruction>>& instructions) {
    for(size_t i = 0; i < instructions.size(); i++) {
        if(instructions.at(i)->m_type != Instruction::Label) 
            continue;
        if(((Label*)instructions.at(i).get())->m_id == label)
            return i;
    }
    return -1;
}

bool isJump(const Instruction* i) {
    return i->m_type == Instruction::Jump || i->m_type == Instruction::JumpFalse || i->m_type == Instruction::JumpTrue;
}

std::shared_ptr<Operation> Builder::operation(Operation::Type t, const TypeInfo& ti, FunctionContext& ictx, std::shared_ptr<Operand> o1, std::shared_ptr<Operand> o2) { 
    if(t != Operation::Reference && t != Operation::Load) {
        if(o1->m_type == Operand::Type::Address) {
            auto reg = makeOrGetRegister(Utils::uniqueRegisterName(), ictx);
            assign(reg, load(o1, ictx), {0,0}, ictx);
            o1 = reg;
        }
        if(o2 && o2->m_type == Operand::Type::Address) {
            auto reg = makeOrGetRegister(Utils::uniqueRegisterName(), ictx);
            assign(reg, load(o2, ictx), {0,0}, ictx);
            o2 = reg;
        }
    }
    
    return std::make_shared<Operation>(t, ti, o1, o2, makeOrGetRegister("mem", ictx));
}

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

    if(!f.m_external) {
        f.m_blocks = std::move(buildCFG(f, ctx));
        if(context.m_optLevel > 0) {
            while(true) {
                bool changed = false;
                changed |= CopyPropagator::propagate(f);
                changed |= CSE::run(f);
                if(!changed) break;
            }
        }
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

    auto label = std::make_shared<Label>(node->m_rangeEnd, Utils::uniqueLabelName());

    if(node->m_retain)
        ictx.m_function->m_retains.insert({reg->m_originalId, reg});
    else if(!reg->m_typeInfo.m_builtin)
        ictx.m_instructions.push_back(std::make_shared<Allocate>(node->m_rangeBegin, reg, reg->m_typeInfo.m_size));

    if(node->m_value) {
        if(node->m_retain) {
            auto guard = makeOrGetRegister("%" + node->m_name.m_name + "_" + std::to_string(ictx.m_scopeDepth) + "_guard", ictx);
            guard->m_typeInfo = context.getTypeInfo(BOOL_T).value();
            guard->m_scale = 1;
            guard->m_typeInfo.m_builtin = true;
            ictx.m_function->m_retains.insert({guard->m_originalId, guard});
            ictx.m_instructions.push_back(
                std::make_shared<Jump>(Instruction::JumpTrue, node->m_rangeBegin, label, guard)
            );
            auto v = std::make_shared<Immediate>();
            v->m_value = true;
            assign(guard, load(v, ictx), node->m_rangeBegin, ictx);
        }
        auto op = buildExpression(node->m_value.get(), context, ictx);
        if(op->m_type == Operand::Type::Immediate && std::holds_alternative<std::string>(std::static_pointer_cast<Immediate>(op)->m_value)) {
            assign(reg, point(op, ictx), node->m_rangeBegin, ictx);
            return;
        }
        assign(reg, load(op, ictx), node->m_rangeBegin, ictx);
        if(node->m_retain)
            ictx.m_instructions.push_back(label);
    }
}

void Builder::build(AST::WhileNode* node, BrawContext& context, FunctionContext& ictx) {
    auto label = std::make_shared<Label>(node->m_condition->m_rangeBegin, Utils::uniqueLabelName() + "_condition");
    auto labelBody = std::make_shared<Label>(node->m_then->m_rangeBegin, Utils::uniqueLabelName() + "_body");
    if(!node->m_do)
        ictx.m_instructions.push_back(std::make_shared<Jump>(Instruction::Jump, node->m_rangeBegin, label));
    ictx.m_instructions.push_back(labelBody);
    build(node->m_then.get(), context, ictx);
    ictx.m_instructions.push_back(label);
    auto condition = buildExpression(node->m_condition.get(), context, ictx);
    ictx.m_instructions.push_back(
        std::make_shared<Jump>(Instruction::JumpTrue, node->m_rangeEnd, labelBody, condition)
    );
}

void Builder::build(AST::ForNode* node, BrawContext& context, FunctionContext& ictx) {
    build(node->m_initializer.get(), context, ictx);
    auto labelCondition = std::make_shared<Label>(node->m_condition->m_rangeBegin, Utils::uniqueLabelName() + "_condition");
    auto labelBody = std::make_shared<Label>(node->m_body->m_rangeBegin, Utils::uniqueLabelName() + "_body");
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
    auto label = std::make_shared<Label>(node->m_rangeBegin, Utils::uniqueLabelName());
    ictx.m_instructions.push_back(
        std::make_shared<Jump>(Instruction::JumpFalse, node->m_condition->m_rangeBegin, label, condition)
    );
    build(node->m_then.get(), context, ictx);
    if(node->m_else) {
        auto elseLabel = std::make_shared<Label>(node->m_else->m_rangeBegin, Utils::uniqueLabelName() + "_else");
        ictx.m_instructions.push_back(
            std::make_shared<Jump>(Instruction::Jump, node->m_condition->m_rangeBegin, elseLabel)
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
            assign(ictx.m_returnRegister, point(op, ictx), node->m_rangeBegin, ictx);
        else
            assign(ictx.m_returnRegister, load(op, ictx), node->m_rangeBegin, ictx);
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
        assign(left, point(right, ictx), node->m_rangeBegin, ictx);
        return;
    }

    assign(left, load(right, ictx), node->m_rangeBegin, ictx);
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
        std::string name = Utils::uniqueRegisterName();
        std::shared_ptr<Register> target = makeOrGetRegister(name, ictx);
        auto label = std::make_shared<Label>(node->m_right->m_rangeEnd, Utils::uniqueLabelName());
        auto left = buildExpression(node->m_left.get(), context, ictx);
        assign(target, load(left, ictx), node->m_left->m_rangeEnd, ictx);
        ictx.m_instructions.push_back(std::make_shared<Jump>(Instruction::JumpTrue, node->m_right->m_rangeEnd, label, target));
        auto right = buildExpression(node->m_right.get(), context, ictx);
        auto orOp = std::make_shared<Operation>(Operation::Or, right->m_typeInfo, target, right);
        assign(target, orOp, node->m_right->m_rangeEnd, ictx);
        ictx.m_instructions.push_back(label);
        return target;
    }

    auto left = buildExpression(node->m_left.get(), context, ictx);
    auto right = buildExpression(node->m_right.get(), context, ictx);

    std::string name = Utils::uniqueRegisterName();
    std::shared_ptr<Register> target = makeOrGetRegister(name, ictx);

    if(node->m_operator == "+")
        assign(target, operation(Operation::Add, left->m_typeInfo, ictx, left, right), node->m_rangeBegin, ictx);
    else if(node->m_operator == "*")
        assign(target, operation(Operation::Multiply, left->m_typeInfo, ictx, left, right), node->m_rangeBegin, ictx);
    else if(node->m_operator == "/")
        assign(target, operation(Operation::Divide, left->m_typeInfo, ictx, left, right), node->m_rangeBegin, ictx);
    else if(node->m_operator == "%")
        assign(target, operation(Operation::Modulo, left->m_typeInfo, ictx, left, right), node->m_rangeBegin, ictx);
    else if(node->m_operator == "-")
        assign(target, operation(Operation::Subtract, left->m_typeInfo, ictx, left, right), node->m_rangeBegin, ictx);
    else if(node->m_operator == "==")
        assign(target, operation(Operation::CompareEquals, context.getTypeInfo(left->m_typeInfo.m_operators.at("==").m_returnType).value(), ictx, left, right), node->m_rangeBegin, ictx);
    else if(node->m_operator == "!=")
        assign(target, operation(Operation::CompareNotEquals, context.getTypeInfo(left->m_typeInfo.m_operators.at("!=").m_returnType).value(), ictx, left, right), node->m_rangeBegin, ictx);
    else if(node->m_operator == ">")
        assign(target, operation(Operation::CompareGreater, context.getTypeInfo(left->m_typeInfo.m_operators.at(">").m_returnType).value(), ictx, left, right), node->m_rangeBegin, ictx);
    else if(node->m_operator == "<")
        assign(target, operation(Operation::CompareLess, context.getTypeInfo(left->m_typeInfo.m_operators.at("<").m_returnType).value(), ictx, left, right), node->m_rangeBegin, ictx);
    else if(node->m_operator == "<=")
        assign(target, operation(Operation::CompareLessEquals, context.getTypeInfo(left->m_typeInfo.m_operators.at("<=").m_returnType).value(), ictx, left, right), node->m_rangeBegin, ictx);
    else if(node->m_operator == ">=")
        assign(target, operation(Operation::CompareGreaterEquals, context.getTypeInfo(left->m_typeInfo.m_operators.at(">=").m_returnType).value(), ictx, left, right), node->m_rangeBegin, ictx);
    else if(node->m_operator == "&" || node->m_operator == "&&")
        assign(target, operation(Operation::And, left->m_typeInfo, ictx, left, right), node->m_rangeBegin, ictx);
    else if(node->m_operator == "|")
        assign(target, operation(Operation::Or, left->m_typeInfo, ictx, left, right), node->m_rangeBegin, ictx);
    else if(node->m_operator == "^")
        assign(target, operation(Operation::Xor, left->m_typeInfo, ictx, left, right), node->m_rangeBegin, ictx);
    
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
    auto ret = makeOrGetRegister(Utils::uniqueRegisterName(), ictx);
    ret->m_typeInfo = Utils::getRawType(op->m_typeInfo, context).value();
    assign(ret, operation(Operation::Dereference, ret->m_typeInfo, ictx, op), node->m_rangeBegin, ictx);
    return ret;
}

std::shared_ptr<Operand> Builder::addressOperator(AST::UnaryOperatorNode* node, std::shared_ptr<Operand> op, BrawContext& context, FunctionContext& ictx) {
    auto ret = makeOrGetRegister(Utils::uniqueRegisterName(), ictx);
    ret->m_typeInfo = Utils::makePointer(op->m_typeInfo);
    assign(ret, point(op, ictx), node->m_rangeBegin, ictx);
    return ret;
}

std::shared_ptr<Operand> Builder::subscriptOperator(AST::UnaryOperatorNode* node, std::shared_ptr<Operand> op, BrawContext& context, FunctionContext& ictx) {
    auto index = buildExpression(node->m_expression.get(), context, ictx);
    if(index->m_type != Operand::Register) {
        auto tmp = makeOrGetRegister(Utils::uniqueRegisterName(), ictx);
        tmp->m_typeInfo = index->m_typeInfo;
        assign(tmp, load(index, ictx), node->m_rangeBegin, ictx);
        index = tmp;
    }
    if(op->m_type != Operand::Register) {
        auto tmp = makeOrGetRegister(Utils::uniqueRegisterName(), ictx);
        tmp->m_typeInfo = op->m_typeInfo;
        if(op->m_type == Operand::Address && cast<Address>(op)->m_scaleSize > 1)
            assign(tmp, point(op, ictx), node->m_rangeBegin, ictx);
        else
            assign(tmp, load(op, ictx), node->m_rangeBegin, ictx);
        op = tmp;
    }

    if(index->m_typeInfo.m_name == INT_T) {
        auto tmp = makeOrGetRegister(Utils::uniqueRegisterName(), ictx);
        assign(tmp, operation(Operation::Upsize, context.getTypeInfo(LONG_T).value(), ictx, index), node->m_rangeBegin, ictx);
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
                ret = makeOrGetRegister(Utils::uniqueRegisterName(), ictx);
                assign(ret, operation(Operation::Upsize, context.getTypeInfo(node->m_data).value(), ictx, op), node->m_rangeBegin, ictx);
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
                ret = makeOrGetRegister(Utils::uniqueRegisterName(), ictx);
                assign(ret, operation(Operation::Upsize, context.getTypeInfo(node->m_data).value(), ictx, op), node->m_rangeBegin, ictx);
            }
        }
        else if(op->m_typeInfo.m_name == LONG_T) {
            if(op->m_type == Operand::Immediate) {
                auto imm = cast<Immediate>(op);
                imm->m_value = (int)std::get<long>(imm->m_value);
                ret = imm;
            }
            else {
                ret = makeOrGetRegister(Utils::uniqueRegisterName(), ictx);
                assign(ret, operation(Operation::Downsize, context.getTypeInfo(node->m_data).value(), ictx, op), node->m_rangeBegin, ictx);
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
                ret = makeOrGetRegister(Utils::uniqueRegisterName(), ictx);
                assign(ret, operation(Operation::Downsize, context.getTypeInfo(node->m_data).value(), ictx, op), node->m_rangeBegin, ictx);
            }
        }
        else if(op->m_typeInfo.m_name == LONG_T) {
            if(op->m_type == Operand::Immediate) {
                auto imm = cast<Immediate>(op);
                imm->m_value = (char)std::get<long>(imm->m_value);
                ret = imm;
            }
            else {
                ret = makeOrGetRegister(Utils::uniqueRegisterName(), ictx);
                assign(ret, operation(Operation::Downsize, context.getTypeInfo(node->m_data).value(), ictx, op), node->m_rangeBegin, ictx);
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
    auto ret = makeOrGetRegister(Utils::uniqueRegisterName(), ictx);
    assign(ret, operation(Operation::LogicalNot, context.getTypeInfo(BOOL_T).value(), ictx, op), node->m_rangeBegin, ictx);
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

    std::string name = Utils::uniqueRegisterName();

    auto fun = context.getFunction(node->m_name, tmpTypes);
    call->m_returnType = fun->m_returnType;
    if(fun->m_returnType.m_size != 0) {
        call->m_optReturn = makeOrGetRegister(name, ictx);
        call->m_optReturn->m_typeInfo = fun->m_returnType;
    }

    ictx.m_instructions.push_back(call);
    return makeOrGetRegister(name, ictx);
}


void Builder::assign(std::shared_ptr<Operand> to, std::shared_ptr<Operation> oper, std::pair<uint32_t, uint32_t> range, FunctionContext& ctx) {
    if(oper->m_type != Operation::Reference) {
        if(oper->m_o1->m_type == Operand::Address) {
            auto tmpReg = makeOrGetRegister(Utils::uniqueRegisterName(), ctx);
            assign(tmpReg, operation(Operation::Reference, oper->m_o1->m_typeInfo, ctx, oper->m_o1), range, ctx);
            oper->m_o1 = tmpReg;
        }
        if(oper->m_o2 && oper->m_o2->m_type == Operand::Address) {
            auto tmpReg = makeOrGetRegister(Utils::uniqueRegisterName(), ctx);
            assign(tmpReg, operation(Operation::Reference, oper->m_o2->m_typeInfo, ctx, oper->m_o2), range, ctx);
            oper->m_o2 = tmpReg;
        }
    }
    
    std::string potentialName = Utils::uniqueRegisterName();
    auto instr = std::make_shared<Assignment>(range);
    if(to->m_type == Operand::Address) {
        instr->m_to = makeOrGetRegister(Utils::uniqueRegisterName(), ctx);
        assign(makeOrGetRegister(potentialName, ctx), operation(Operation::Reference, to->m_typeInfo, ctx, to), range, ctx);
    }
    else
        instr->m_to = to;

    if(instr->m_to->m_typeInfo.m_name == "")
        instr->m_to->m_typeInfo = oper->m_typeInfo;
    instr->m_operation = oper;
    ctx.m_instructions.push_back(instr);

    if(oper->m_type == Operation::Point) {
        cast<Register>(oper->m_o1)->m_memoryDependant = true;
    }
    if(oper->m_type == Operation::Reference) {
        cast<Register>(instr->m_to)->m_memoryDependant = true;
    }
    else if(oper->m_type == Operation::Point || oper->m_type == Operation::Load || oper->m_type == Operation::Dereference || oper->m_type == Operation::PartialDereference) {
        if(cast<Register>(oper->m_o1)->m_type == Operand::Register)
            cast<Register>(instr->m_to)->m_memoryDependant = cast<Register>(oper->m_o1)->m_memoryDependant;
        if(oper->m_o2 && cast<Register>(oper->m_o2)->m_type == Operand::Register)
            cast<Register>(instr->m_to)->m_memoryDependant |= cast<Register>(oper->m_o2)->m_memoryDependant;
    }

    if(to->m_type == Operand::Address) {
        auto writeMem = std::make_shared<WriteMem>(range, makeOrGetRegister(potentialName, ctx), instr->m_to);
        writeMem->m_memory = makeOrGetRegister("mem", ctx);
        ctx.m_instructions.push_back(writeMem);
    }
}

std::shared_ptr<Register> Builder::makeOrGetRegister(const std::string& name, FunctionContext& ctx) {
    if(ctx.m_registers.contains(name))
        return ctx.m_registers[name];

    std::shared_ptr<Register> reg = std::make_shared<Register>(name);
    ctx.m_registers[name] = reg;
    return reg;
}

std::shared_ptr<Operation> Builder::load(std::shared_ptr<Operand> op, FunctionContext& ictx) {
    return operation(Operation::Load, op->m_typeInfo, ictx, op);
}

std::shared_ptr<Operation> Builder::point(std::shared_ptr<Operand> op, FunctionContext& ictx) {
    return operation(Operation::Point, Utils::makePointer(op->m_typeInfo), ictx, op);
}


std::vector<std::shared_ptr<Block>> Builder::buildCFG(Function& f, FunctionContext& context) {
    std::vector<std::shared_ptr<Block>> blocks = getBlocks(f);

    std::vector<std::shared_ptr<Block>> currentPath;
    std::unordered_map<std::shared_ptr<Block>, std::vector<std::vector<std::shared_ptr<Block>>>> paths;
    //compute paths
    getAllPaths(blocks.at(0), currentPath, paths);
    //compute dominators
    for(auto& block : blocks)
        block->m_dominators.insert(block->m_dominators.begin(), blocks.begin(), blocks.end());
    blocks.at(0)->m_dominators.clear(); blocks.at(0)->m_dominators.push_back(blocks.at(0));
    bool changed = false;
    do {
        changed = false;
        for(size_t i = 1; i < blocks.size(); i++) {
            std::shared_ptr<Block> b = blocks.at(i);
            std::unordered_map<std::shared_ptr<Block>, size_t> appearence;
            std::vector<std::shared_ptr<Block>> ordered;
            std::vector<std::shared_ptr<Block>> newDominators;

            for(auto& predecessor : b->m_predecessors) {
                for(auto d : predecessor->m_dominators) {
                    if(!appearence.contains(d)) {
                        appearence.insert({d, 0});
                        ordered.push_back(d);
                    }
                    appearence[d]++;
                }
            }

            for(auto ord : ordered) {
                if(appearence.at(ord) != b->m_predecessors.size())
                    continue;
                newDominators.push_back(ord);
            }
            newDominators.push_back(b);

            if(newDominators != b->m_dominators) {
                b->m_dominators = std::move(newDominators);
                changed = true;
            }
        }
    }
    while(changed);

    for(auto block : blocks) {
        for(auto dominator : block->m_dominators) {
            dominator->m_dominated.push_back(block);
        }
    }
    
    //compute dominance frontiers
    for(auto block : blocks) {
        if(block->m_predecessors.size() < 2)
            continue;
        for(auto predecessor : block->m_predecessors) {
            auto runner = predecessor;
            auto idom = block->getImmediateDomiator();
            while(runner && runner != idom) {
                static size_t what = 0;
                what++;
                if(what > 1000)
                    std::cout << "a";
                runner->m_dominanceFrontiers.insert(block);
                runner = runner->getImmediateDomiator();
            }
        }
    }

    std::unordered_map<std::string, std::pair<std::shared_ptr<Operand>, std::vector<std::shared_ptr<Block>>>> blocksThatAssignVariable;
    for(auto& block : blocks) {
        for(size_t i = block->m_instructionRange.first; i <= block->m_instructionRange.second; i++) {
            if(f.m_instructions.at(i)->m_type == Instruction::Assign) {
                const Assignment* a = static_cast<const Assignment*>(f.m_instructions.at(i).get());
                auto& pair = blocksThatAssignVariable[operandString(a->m_to)];
                pair.first = a->m_to;
                pair.second.push_back(block);
            }
            else if(f.m_instructions.at(i)->m_type == Instruction::Call) {
                const Call* c = static_cast<const Call*>(f.m_instructions.at(i).get());
                if(!c->m_optReturn)
                    continue;
                auto& pair = blocksThatAssignVariable[operandString(c->m_optReturn)];
                pair.first = c->m_optReturn;
                pair.second.push_back(block);
            }
            else if(f.m_instructions.at(i)->m_type == Instruction::WriteMem) {
                const WriteMem* w = static_cast<const WriteMem*>(f.m_instructions.at(i).get());
                auto& pair = blocksThatAssignVariable[operandString(w->m_memory)];
                pair.first = w->m_memory;
                pair.second.push_back(block);
            }
        }
    }

    for(auto& pair : blocksThatAssignVariable) {
        if(pair.second.second.size() <= 1)
            continue;
        placePhiBlocks(pair.second.first, pair.second.second, blocks, f);
    }

    std::unordered_map<std::string, size_t> counters;
    std::unordered_map<std::string, std::vector<std::string>> nameStack;
    std::unordered_set<std::shared_ptr<Block>> visited;
    std::unordered_map<std::string, std::shared_ptr<Operand>> nameForOperand;
    rename(blocks.at(0), f, counters, nameStack, visited, nameForOperand, context);
    
    return blocks;
}

std::shared_ptr<Register> cloneRegister(std::shared_ptr<Register> reg) {
    auto ret = std::make_shared<Register>(reg->m_id, reg->m_typeInfo);
    ret->m_memoryDependant = reg->m_memoryDependant;
    ret->m_isPhi = reg->m_isPhi;
    return ret;
}

std::shared_ptr<Operand> replace(std::shared_ptr<Operand> op, std::unordered_map<std::string, std::vector<std::string>>& nameStack, std::unordered_map<std::string, std::shared_ptr<Operand>>& nameForOperand) {
    if(op->m_type == Operand::Address) {
        auto addr = cast<Address>(op);
        addr->m_base = cast<Register>(replace(addr->m_base, nameStack, nameForOperand));
        if(addr->m_index)
            addr->m_index = cast<Register>(replace(addr->m_index, nameStack, nameForOperand));
        return op;
    }
    
    if(op->m_type != Operand::Register)
        return op;
    auto reg = cloneRegister(cast<Register>(op));
    std::string opStr = reg->m_originalId;
    const std::string& name = nameStack.at(opStr).back();
    reg->m_id = name;
    nameForOperand[opStr] = reg;
    return reg;
};

void Builder::rename(std::shared_ptr<Block> block, Function& f, std::unordered_map<std::string, size_t>& counters, std::unordered_map<std::string, std::vector<std::string>>& nameStack, std::unordered_set<std::shared_ptr<Block>>& visited, std::unordered_map<std::string, std::shared_ptr<Operand>>& nameForOperand, FunctionContext& context) {
    if(visited.contains(block))
        return;
    visited.insert(block);
    std::unordered_set<std::string> assignedVariables;


    auto assigned = [&](std::shared_ptr<Operand> op) -> std::shared_ptr<Operand> {
        if(op->m_type != Operand::Register)
            return op;
        auto reg = cloneRegister(cast<Register>(op));
        std::string opStr = reg->m_originalId;
        if(!counters.contains(opStr))
            counters.insert({opStr, 0});
        counters[opStr]++;
        nameStack[opStr].push_back(opStr + ":" + std::to_string(counters[opStr]));
        reg->m_id = nameStack[opStr].back();
        nameForOperand[opStr] = reg;
        assignedVariables.insert(opStr);
        return reg;
    };

    if(!nameStack.contains("mem"))
        assigned(makeOrGetRegister("mem", context));

    for(auto arg : f.m_args)
        assigned(arg);
    
    for(auto& ret : f.m_retains)
        assigned(ret.second);

    for(size_t i = block->m_instructionRange.first; i <= block->m_instructionRange.second; i++) {
        auto instr = f.m_instructions.at(i);
        switch(instr->m_type) {
            case Instruction::Assign: {
                Assignment* a = static_cast<Assignment*>(instr.get());
                a->m_to = assigned(a->m_to);
                if(a->m_operation->m_o1)
                    a->m_operation->m_o1 = replace(a->m_operation->m_o1, nameStack, nameForOperand);
                if(a->m_operation->m_o2)
                    a->m_operation->m_o2 = replace(a->m_operation->m_o2, nameStack, nameForOperand);
                a->m_operation->m_memory = cast<Register>(replace(a->m_operation->m_memory, nameStack, nameForOperand));

                bool mem = false;
                if(a->m_operation->m_o1->m_type == Operand::Register)
                    mem = cast<Register>(a->m_operation->m_o1)->m_memoryDependant;
                if(a->m_operation->m_o2 && a->m_operation->m_o2->m_type == Operand::Register)
                    mem |= cast<Register>(a->m_operation->m_o2)->m_memoryDependant;
                if(mem) {
                    a->m_operation->m_memoryDependant = true;
                }
                break;
            }
            case Instruction::Allocate: {
                Allocate* a = static_cast<Allocate*>(instr.get());
                a->m_to = assigned(a->m_to);
                break;
            }
            case Instruction::Call: {
                Call* c = static_cast<Call*>(instr.get());
                for(auto& argument : c->m_parameters) {
                    argument = replace(argument, nameStack, nameForOperand);
                }
                if(c->m_optReturn)
                    c->m_optReturn = cast<Register>(assigned(c->m_optReturn));
                break;
            }
            case Instruction::JumpFalse:
            case Instruction::JumpTrue: {
                Jump* j = static_cast<Jump*>(instr.get());
                if(j->m_check)
                    j->m_check = replace(j->m_check, nameStack, nameForOperand);
                break;
            }
            case Instruction::WriteMem: {
                WriteMem* w = static_cast<WriteMem*>(instr.get());
                w->m_to = assigned(w->m_to);
                w->m_memory = cast<Register>(assigned(w->m_memory));
                w->m_value = replace(w->m_value, nameStack, nameForOperand);
                break;
            }
            case Instruction::Phi: {
                Phi* p = static_cast<Phi*>(instr.get());
                p->m_to = assigned(p->m_to);
                nameStack[cast<Register>(p->m_to)->m_originalId].back() += "_phi";
                break;
            }
            case Instruction::Return:
            case Instruction::Label:
            case Instruction::Jump:
                break;
        }
    }

    for(auto s : block->m_connections) {
        for(auto& phiPair : s->m_phiForVariable) {
            phiPair.second->m_operands.push_back(nameForOperand.at(phiPair.first));
            Instruction::Type t = f.m_instructions.at(block->m_instructionRange.second)->m_type;
            size_t idx = t == Instruction::Jump || t == Instruction::JumpFalse || t == Instruction::JumpTrue ? block->m_instructionRange.second - 1 : block->m_instructionRange.second;
            phiPair.second->m_placeOpAt.push_back(idx);
        }
    }

    for(auto d : block->m_connections)
        rename(d, f, counters, nameStack, visited, nameForOperand, context);


    for(const std::string& variable : assignedVariables) {
        nameStack[variable].pop_back();
    }
}

std::vector<std::shared_ptr<Block>> Builder::getBlocks(const Function& f) {
    std::vector<std::shared_ptr<Block>> blocks;
    Block* current = nullptr;
    std::unordered_map<size_t, size_t> blockForInstruction;

    for(size_t i = 0; i < f.m_instructions.size(); i++) {
        if(f.m_instructions.at(i)->m_type == Instruction::Label || (i > 0 && isJump(f.m_instructions.at(i - 1).get()))) {
            if(!(f.m_instructions.at(i)->m_type == Instruction::Label && i > 0 && f.m_instructions.at(i - 1)->m_type == Instruction::Label)) {
                blocks.push_back(std::make_shared<Block>());
                current = blocks.at(blocks.size() - 1).get();
                current->m_instructionRange.first = i;
            }
        }
        current->m_instructionRange.second = i;
        blockForInstruction[i] = blocks.size() - 1;
    }
    std::unordered_set<std::shared_ptr<Block>> visited;
    buildGraphRecursive(blocks.at(0), blockForInstruction, blocks, visited, f);
    return blocks;
}

void Builder::buildGraphRecursive(std::shared_ptr<Block> root, const std::unordered_map<size_t, size_t>& blockForInstruction, const std::vector<std::shared_ptr<Block>>& blocks, std::unordered_set<std::shared_ptr<Block>>& visited, const Function& f) {
    if(visited.contains(root))
        return;
    visited.insert(root);

    for(size_t i = root->m_instructionRange.first; i <= root->m_instructionRange.second; i++) {
        if(f.m_instructions.at(i)->m_type == Instruction::Return)
            return;
    }
    
    auto& lastInstruction = f.m_instructions.at(root->m_instructionRange.second);
    if(lastInstruction->m_type == Instruction::JumpFalse || lastInstruction->m_type == Instruction::JumpTrue || lastInstruction->m_type == Instruction::Jump) {
        auto jump = cast<Jump>(lastInstruction);
        std::shared_ptr<Block> next = blocks.at(blockForInstruction.at(getJumpTarget(jump->m_to->m_id, f.m_instructions)));
        root->m_connections.push_back(next);
        next->m_predecessors.push_back(root);
        buildGraphRecursive(next, blockForInstruction, blocks, visited, f);
        if(lastInstruction->m_type == Instruction::Jump)
            return;
    }

    if(blockForInstruction.contains(root->m_instructionRange.second + 1)) {
        std::shared_ptr<Block> next = blocks.at(blockForInstruction.at(root->m_instructionRange.second + 1));
        root->m_connections.push_back(next);
        next->m_predecessors.push_back(root);
        buildGraphRecursive(next, blockForInstruction, blocks, visited, f);
    }
}

void Builder::getAllPaths(std::shared_ptr<Block> root, std::vector<std::shared_ptr<Block>>& currentPath, std::unordered_map<std::shared_ptr<Block>, std::vector<std::vector<std::shared_ptr<Block>>>>& paths) {
    if(std::find(currentPath.begin(), currentPath.end(), root) != currentPath.end())
        return;
    currentPath.push_back(root);
    std::vector<std::shared_ptr<Block>> pathVec; pathVec.reserve(currentPath.size());
    for(auto& b : currentPath)
        pathVec.push_back(b);
    if(pathVec.size() > 0)
        paths[root].push_back(std::move(pathVec));

    for(auto& con : root->m_connections)
        getAllPaths(con, currentPath, paths);

    currentPath.pop_back();
}

std::string Builder::operandString(std::shared_ptr<Operand> op) {
    switch(op->m_type) {
        case Operand::Register: {
            const Register* reg = static_cast<const Register*>(op.get());
            return reg->m_id;
        }
        case Operand::Immediate: {
            const Immediate* imm = static_cast<const Immediate*>(op.get());
            switch(imm->m_value.index()) {
            case 0:
                return std::to_string(std::get<int>(imm->m_value));
            case 1:
                return std::to_string(std::get<long>(imm->m_value));
            case 2:
                return std::to_string(std::get<float>(imm->m_value));
            case 3:
                return std::to_string(std::get<double>(imm->m_value));
            case 4:
                return std::to_string(std::get<bool>(imm->m_value));
            case 5:
                return std::get<std::string>(imm->m_value);
            case 6:
                return "NULL";
            default:
                return "";
            }
            break;
        }
        case Operand::Address: {
            const Address* add = static_cast<const Address*>(op.get());
            if(add->m_index)
                return "[" + add->m_base->m_id + "+" + std::to_string(add->m_scale) + "*" + add->m_index->m_id + "+" + std::to_string(add->m_offset) + "]";
            else
                return "[" + add->m_base->m_id + "+" + std::to_string(add->m_offset) + "]";
            break;
        }
    }
}

void Builder::placePhiBlocks(std::shared_ptr<Operand> op, std::vector<std::shared_ptr<Block>> blocks, const std::vector<std::shared_ptr<Block>>& allBlocks, Function& f) {
    std::unordered_set<std::shared_ptr<Block>> visited;
    while(blocks.size() > 0) {
        std::shared_ptr<Block> block = blocks.back();
        blocks.pop_back();
        for(auto& frontier : block->m_dominanceFrontiers) {
            if(frontier->m_phiForVariable.contains(operandString(op)))
                continue;
            auto phi = std::make_shared<Phi>(op);
            frontier->m_phiForVariable[operandString(op)] = phi;
            f.m_instructions.insert(f.m_instructions.begin() + frontier->m_instructionRange.first + 1, phi);
            if (visited.insert(frontier).second) {
                blocks.push_back(frontier);
            }
            for(auto& b : allBlocks) {
                if(b == frontier)
                    continue;
                if(b->m_instructionRange.first > frontier->m_instructionRange.first + 1)
                    b->m_instructionRange.first++;
                if(b->m_instructionRange.second > frontier->m_instructionRange.first + 1)
                    b->m_instructionRange.second++;
            }
            frontier->m_instructionRange.second++;
        }
    }
}

}