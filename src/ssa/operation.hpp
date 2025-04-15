#pragma once

#include "ssa/operand.hpp"
#include "type_info.hpp"

namespace SSA {

struct Operation {
    enum Type {
        Add,
        Subtract,
        Multiply,
        Divide,
        Point,
        Dereference,
        PartialDereference,
        Upsize,
        Downsize,
        CompareEquals,
        CompareNotEquals,
        CompareGreaterEquals,
        CompareLessEquals,
        CompareGreater,
        CompareLess,
        Modulo,
        And,
        Or,
        Xor,
        LogicalNot,
        Load,
        Reference
    };

    Operation(Type t, TypeInfo ti, std::shared_ptr<Operand> o1 = nullptr, std::shared_ptr<Operand> o2 = nullptr) : m_type(t), m_typeInfo(ti), m_o1(o1), m_o2(o2) {}

    Type m_type;
    TypeInfo m_typeInfo;
    std::shared_ptr<Operand> m_o1;
    std::shared_ptr<Operand> m_o2;
};

}