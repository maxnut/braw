#include "builder_ssa.hpp"
#include "ir/copy_propagator.hpp"
#include "ir/instruction.hpp"
#include "ir/instructions/basic.hpp"
#include "ir/instructions/call.hpp"
#include "ir/label.hpp"
#include "ir/value.hpp"
#include "rules.hpp"
#include "ssa/instruction.hpp"
#include "ssa/operand.hpp"
#include "ssa/operation.hpp"
#include "utils.hpp"
#include "ssa/block.hpp"

#include <array>
#include <memory>
#include <vector>

File IRBuilderSSA::build(const SSA::File& file, BrawContext& context) {
    File fRet;
    fRet.m_path = file.m_path;

    for(auto& function : file.m_functions)
        fRet.m_functions.emplace_back(std::move(build(function, context)));

    for(auto& function : fRet.m_functions) {
        if(function.m_external)
            fRet.m_externals.push_back(&function);
    }

    return fRet;
}

bool operandEquals(Operand op1, Operand op2) {
    if(op1.index() != op2.index())
        return false;
    switch(op1.index()) {
        case 1:
            return std::get<std::shared_ptr<Register>>(op1) == std::get<std::shared_ptr<Register>>(op2);
        case 2:
            return std::get<Value>(op1) == std::get<Value>(op2);
        case 3: {
            auto addr1 = std::get<Address>(op1);
            auto addr2 = std::get<Address>(op2);
            return operandEquals(addr1.m_base, addr2.m_base)
                && addr1.m_offset == addr2.m_offset
                && addr1.m_scale == addr2.m_scale
                && operandEquals(addr1.m_index, addr2.m_index);
        }
        default:
            break;
    }
    return false;
}

Function IRBuilderSSA::build(const SSA::Function& function, BrawContext& context) {
    Function fRet;
    IRFunctionContextSSA ictx{context};
    ictx.m_function = &fRet;

    fRet.m_name = function.m_name;
    fRet.m_external = function.m_external;
    if(function.m_optReturn)
        fRet.m_optReturn = std::get<std::shared_ptr<Register>>(convertOperand(function.m_optReturn.get(), ictx));
    for(auto arg : function.m_args)
        fRet.m_args.push_back(std::get<std::shared_ptr<Register>>(convertOperand(arg.get(), ictx)));
    for(auto ret : function.m_retains)
        fRet.m_retains.insert({ret.first, std::get<std::shared_ptr<Register>>(convertOperand(ret.second.get(), ictx))});

    for(size_t i = 0; i < function.m_instructions.size(); i++) {
        auto& instruction = function.m_instructions.at(i);
        build(instruction.get(), ictx);
        ictx.m_ssaToIrIdx.insert({i, fRet.m_instructions.size() - 1});
    }

    for(auto block : function.m_blocks) {
        for(auto& pair : block->m_phiForVariable)
            build(pair.second.get(), ictx);
    }

    // if(!fRet.m_external && context.m_optLevel > 0)
    //     CopyPropagator::propagate(fRet);

    std::erase_if(fRet.m_instructions, [](std::unique_ptr<Instruction>& instr){
        if(instr->m_type != Instruction::Move && instr->m_type != Instruction::Copy)
            return false;
        auto basic = static_cast<BasicInstruction*>(instr.get());
        if(operandEquals(basic->m_o1, basic->m_o2))
            return true;
        return false;
    });

    return fRet;
}

void IRBuilderSSA::buildAssignment(const SSA::Assignment* assignment, Instruction::Type type, IRFunctionContextSSA& ictx) {
    auto left = convertOperand(assignment->m_operation->m_o1.get(), ictx);
    auto target = std::get<std::shared_ptr<Register>>(convertOperand(assignment->m_to.get(), ictx));
    if(assignment->m_operation->m_o2) {
        auto right = convertOperand(assignment->m_operation->m_o2.get(), ictx);
        moveToRegister(target->m_id, left, assignment->m_range, ictx);
        left = right;
    }
    ictx.m_function->m_instructions.push_back(std::make_unique<BasicInstruction>(type, assignment->m_range, target, left));
}

void IRBuilderSSA::buildCompare(const SSA::Assignment* assignment, Instruction::Type type, IRFunctionContextSSA& ictx) {
    auto left = convertOperand(assignment->m_operation->m_o1.get(), ictx);
    auto right = convertOperand(assignment->m_operation->m_o2.get(), ictx);
    auto target = std::get<std::shared_ptr<Register>>(convertOperand(assignment->m_to.get(), ictx));
    ictx.m_function->m_instructions.push_back(std::make_unique<BasicInstruction>(type, assignment->m_range, left, right, target));
}

void IRBuilderSSA::build(const SSA::Instruction* instruction, IRFunctionContextSSA& ictx) {
    switch (instruction->m_type) {
        case SSA::Instruction::Assign:
            build(static_cast<const SSA::Assignment*>(instruction), ictx);
            break;
        case SSA::Instruction::Allocate:
            build(static_cast<const SSA::Allocate*>(instruction), ictx);
            break;
        case SSA::Instruction::Call:
            build(static_cast<const SSA::Call*>(instruction), ictx);
            break;
        case SSA::Instruction::Return:
            ictx.m_function->m_instructions.push_back(std::make_unique<Instruction>(Instruction::Type::Return, instruction->m_range));
            break;
        case SSA::Instruction::JumpFalse:
        case SSA::Instruction::JumpTrue:
        case SSA::Instruction::Jump:
            build(static_cast<const SSA::Jump*>(instruction), ictx);
            break;
        case SSA::Instruction::Label:
            build(static_cast<const SSA::Label*>(instruction), ictx);
            break;
        case SSA::Instruction::Phi:
            // build(static_cast<const SSA::Phi*>(instruction), ictx);
            break;
        case SSA::Instruction::WriteMem:
            build(static_cast<const SSA::WriteMem*>(instruction), ictx);
            break;
    }
}


void IRBuilderSSA::build(const SSA::Assignment* assignment, IRFunctionContextSSA& context) {
    SSA::Operation* operation = assignment->m_operation.get();
    switch(operation->m_type) {
        case SSA::Operation::Add:
            buildAssignment(assignment, Instruction::Type::Add, context);
            break;
        case SSA::Operation::Subtract:
            buildAssignment(assignment, Instruction::Type::Subtract, context);
            break;
        case SSA::Operation::Multiply:
            buildAssignment(assignment, Instruction::Type::Multiply, context);
            break;
        case SSA::Operation::Divide:
            buildAssignment(assignment, Instruction::Type::Divide, context);
            break;
        case SSA::Operation::Point:
            buildAssignment(assignment, Instruction::Type::Point, context);
            break;
        case SSA::Operation::Dereference:
            buildAssignment(assignment, Instruction::Type::Dereference, context);
            break;
        case SSA::Operation::PartialDereference:
            buildAssignment(assignment, Instruction::Type::PartialDereference, context);
            break;
        case SSA::Operation::Upsize:
            buildAssignment(assignment, Instruction::Type::Upsize, context);
            break;
        case SSA::Operation::Downsize:
            buildAssignment(assignment, Instruction::Type::Downsize, context);
            break;
        case SSA::Operation::CompareEquals:
            buildCompare(assignment, Instruction::Type::CompareEquals, context);
            break;
        case SSA::Operation::CompareNotEquals:
            buildCompare(assignment, Instruction::Type::CompareNotEquals, context);
            break;
        case SSA::Operation::CompareGreaterEquals:
            buildCompare(assignment, Instruction::Type::CompareGreaterEquals, context);
            break;
        case SSA::Operation::CompareLessEquals:
            buildCompare(assignment, Instruction::Type::CompareLessEquals, context);
            break;
        case SSA::Operation::CompareGreater:
            buildCompare(assignment, Instruction::Type::CompareGreater, context);
            break;
        case SSA::Operation::CompareLess:
            buildCompare(assignment, Instruction::Type::CompareLess, context);
            break;
        case SSA::Operation::Modulo:
            buildAssignment(assignment, Instruction::Type::Modulo, context);
            break;
        case SSA::Operation::And:
            buildAssignment(assignment, Instruction::Type::And, context);
            break;
        case SSA::Operation::Or:
            buildAssignment(assignment, Instruction::Type::Or, context);
            break;
        case SSA::Operation::Xor:
            buildAssignment(assignment, Instruction::Type::Xor, context);
            break;
        case SSA::Operation::LogicalNot:
            buildAssignment(assignment, Instruction::Type::LogicalNot, context);
            break;
        case SSA::Operation::Reference: {
            context.m_refs.insert({std::static_pointer_cast<SSA::Register>(assignment->m_to)->m_originalId, std::get<Address>(convertOperand(assignment->m_operation->m_o1.get(), context))});
            break;
        }
        case SSA::Operation::Load: {
            auto left = convertOperand(assignment->m_operation->m_o1.get(), context);
            auto target = std::get<std::shared_ptr<Register>>(convertOperand(assignment->m_to.get(), context));

            moveToRegister(target->m_id, left, assignment->m_range, context);
            break;
        }
    }
}

void IRBuilderSSA::build(const SSA::Allocate* allocate, IRFunctionContextSSA& context) {
    context.m_function->m_instructions.push_back(std::make_unique<BasicInstruction>(Instruction::Allocate, allocate->m_range, convertOperand(allocate->m_to.get(), context), Value((long)allocate->m_size)));
}

void IRBuilderSSA::build(const SSA::Call* call, IRFunctionContextSSA& context) {
    auto callRet = std::make_unique<CallInstruction>(call->m_range);
    callRet->m_id = call->m_id;
    if(call->m_optReturn)
        callRet->m_optReturn = std::get<std::shared_ptr<Register>>(convertOperand(call->m_optReturn.get(), context));

    callRet->m_returnType = call->m_returnType;

    for(auto param : call->m_parameters)
        callRet->m_parameters.push_back(convertOperand(param.get(), context));
    context.m_function->m_instructions.push_back(std::move(callRet));
}
void IRBuilderSSA::build(const SSA::Jump* jump, IRFunctionContextSSA& context) {
    auto type = jump->m_type == SSA::Instruction::Jump ? Instruction::Jump : jump->m_type == SSA::Instruction::JumpTrue ? Instruction::JumpTrue : Instruction::JumpFalse;
    auto basic = std::make_unique<BasicInstruction>(type, jump->m_range);
    Label l{jump->m_to->m_range};
    l.m_id = jump->m_to->m_id;
    if(type == Instruction::JumpFalse || type == Instruction::JumpTrue) {
        basic->m_o1 = convertOperand(jump->m_check.get(), context);
        basic->m_o2 = l;
        context.m_function->m_instructions.push_back(std::move(basic));
        return;
    }
    basic->m_o1 = l;
    context.m_function->m_instructions.push_back(std::move(basic));
}
void IRBuilderSSA::build(const SSA::Label* label, IRFunctionContextSSA& context) {
    auto l = std::make_unique<Label>(label->m_range);
    l->m_id = label->m_id;
    context.m_function->m_instructions.push_back(std::move(l));
}

void IRBuilderSSA::build(const SSA::Phi* phi, IRFunctionContextSSA& context) {
    auto to = std::get<std::shared_ptr<Register>>(convertOperand(phi->m_to.get(), context));
    for(size_t i = 0; i < phi->m_operands.size(); i++) {
        auto op = convertOperand(phi->m_operands.at(i).get(), context);

        if(to->m_type.m_name == "")
            to->m_type = getOperandType(op, context);
        to->m_registerType = getRegisterType(to->m_type);
        Instruction::Type instrType = to->m_registerType == RegisterType::Struct ? Instruction::Copy : Instruction::Move;
        auto instr = std::make_unique<BasicInstruction>(instrType, phi->m_range, to, op);
        size_t idx = context.m_ssaToIrIdx.at(phi->m_placeOpAt.at(i)) + 1;
        context.m_function->m_instructions.insert(context.m_function->m_instructions.begin() + idx, std::move(instr));

        for(auto& pair : context.m_ssaToIrIdx) {
            if(pair.second > idx)
                pair.second++;
        }
    }
}

void IRBuilderSSA::build(const SSA::WriteMem* writeMem, IRFunctionContextSSA& context) {
    auto reg = std::static_pointer_cast<SSA::Register>(writeMem->m_to);
    Address left = context.m_refs.at(reg->m_originalId);
    auto right = convertOperand(writeMem->m_value.get(), context);
    context.m_function->m_instructions.push_back(std::make_unique<BasicInstruction>(left.m_typeInfo.m_builtin || Rules::isPtr(left.m_typeInfo.m_name) ? Instruction::Move : Instruction::Copy, writeMem->m_range, left, right));
}


Operand IRBuilderSSA::convertOperand(const SSA::Operand* operand, IRFunctionContextSSA& context) {
    switch (operand->m_type) {
        case SSA::Operand::Register: {
            const SSA::Register* reg = static_cast<const SSA::Register*>(operand);
            if(context.m_refs.contains(reg->m_originalId))
                return context.m_refs.at(reg->m_originalId);
            std::string id = reg->m_id;
            if(id.contains(":"))
                id = id.substr(0, id.find(":"));
            std::shared_ptr<Register> rReg = makeOrGetRegister(id, context);    
            rReg->m_type = reg->m_typeInfo;
            rReg->m_registerType = getRegisterType(reg->m_typeInfo);
            rReg->m_scale = reg->m_scale;
            return rReg;
        }
        case SSA::Operand::Immediate:
            return static_cast<const SSA::Immediate*>(operand)->m_value;
        case SSA::Operand::Address: {
            const SSA::Address* addr = static_cast<const SSA::Address*>(operand);   
            Address ret;
            ret.m_base = std::get<std::shared_ptr<Register>>(convertOperand(addr->m_base.get(), context));
            ret.m_offset = addr->m_offset;
            ret.m_typeInfo = addr->m_typeInfo;
            if(addr->m_index)
                ret.m_index = std::get<std::shared_ptr<Register>>(convertOperand(addr->m_index.get(), context));
            ret.m_scale = addr->m_scale;
            ret.m_scaleSize = addr->m_scaleSize;
            return ret;
        }
    }
    return 0;
}

RegisterType IRBuilderSSA::getRegisterType(const TypeInfo& type) {
    if(type.m_name == INT_T)
        return RegisterType::Signed;
    else if(type.m_name == LONG_T)
        return RegisterType::Signed;
    else if(type.m_name == FLOAT_T)
        return RegisterType::Single;
    else if(type.m_name == DOUBLE_T)
        return RegisterType::Double;
    else if(type.m_name == CHAR_T)
        return RegisterType::Signed;
    else if(type.m_name == BOOL_T)
        return RegisterType::Signed;
    else if(type.m_name == UINT_T)
        return RegisterType::Unsigned;
    else if(type.m_name == ULONG_T)
        return RegisterType::Unsigned;
    else if(type.m_name == UCHAR_T)
        return RegisterType::Unsigned;
    else if(Rules::isPtr(type.m_name)) 
        return RegisterType::Pointer;

    return RegisterType::Struct;
}


TypeInfo IRBuilderSSA::getOperandType(Operand op, IRFunctionContextSSA& ictx) {
    switch(op.index()) {
        case 1: {
            return std::get<std::shared_ptr<Register>>(op)->m_type;
        }
        case 2: {
            std::array<TypeInfo, 7> types = {
                ictx.ctx.getTypeInfo(INT_T).value(), ictx.ctx.getTypeInfo(LONG_T).value(), ictx.ctx.getTypeInfo(FLOAT_T).value(), ictx.ctx.getTypeInfo(DOUBLE_T).value(),
                ictx.ctx.getTypeInfo(BOOL_T).value(), Utils::makePointer(ictx.ctx.getTypeInfo(CHAR_T).value()), Utils::makePointer(ictx.ctx.getTypeInfo(VOID_T).value())
            };
            return types.at(std::get<Value>(op).index());
        }
        case 3: {
            auto addr = std::get<Address>(op);
            return addr.m_typeInfo;
        }
        default:
            break;
    }
    
    return ictx.ctx.getTypeInfo(VOID_T).value();
}

void IRBuilderSSA::moveToRegister(const std::string& name, Operand& op, std::pair<uint32_t, uint32_t> pos, IRFunctionContextSSA& ictx) {
    std::shared_ptr<Register> reg = makeOrGetRegister(name, ictx);

    if(reg->m_type.m_name == "")
        reg->m_type = getOperandType(op, ictx);
    reg->m_registerType = getRegisterType(reg->m_type);
    Instruction::Type instrType = reg->m_registerType == RegisterType::Struct ? Instruction::Copy : Instruction::Move;
    ictx.m_function->m_instructions.push_back(std::make_unique<BasicInstruction>(instrType, pos, reg, op));
}

std::shared_ptr<Register> IRBuilderSSA::makeOrGetRegister(const std::string& name, IRFunctionContextSSA& ictx) {
    if(ictx.m_registers.contains(name))
        return ictx.m_registers[name];

    std::shared_ptr<Register> reg = std::make_shared<Register>(name);
    ictx.m_registers[name] = reg;
    return reg;
}