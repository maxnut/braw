#pragma once

#include <cstddef>
#include <utility>
struct Instruction {
    enum Type {
        Move,
        Point,
        Add,
        Subtract,
        Multiply,
        Divide,
        Call,
        Return,
        CompareEquals,
        CompareNotEquals,
        CompareGreaterEquals,
        CompareLessEquals,
        CompareGreater,
        CompareLess,
        JumpFalse,
        JumpTrue,
        Jump,
        Label,
        Allocate,
        Copy,
        Dereference,
        PartialDereference,
        Upsize,
        Downsize,
        Modulo,
        And,
        Or,
        Xor,
        LogicalNot,
        Count
    };

    Instruction(Type type, std::pair<uint32_t, uint32_t> pos) : m_type(type), m_pos(pos) {}

    Type m_type = Count;
    std::pair<uint32_t, uint32_t> m_pos;
};