#pragma once

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
        Count
    };

    Instruction(Type type) : m_type(type) {}

    Type m_type = Count;
};