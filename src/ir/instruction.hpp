#pragma once

struct Instruction {
    enum Type {
        Move,
        Point,
        Add,
        Subtract,
        Multiply,
        Call,
        Return,
        CompareEquals,
        CompareNotEquals,
        CompareGreaterEquals,
        CompareLessEquals,
        JumpFalse,
        Jump,
        Label,
        Allocate,
        Copy,
        Dereference,
        Count
    };

    Instruction(Type type) : m_type(type) {}

    Type m_type = Count;
};