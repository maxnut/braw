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
        Label,
        Allocate,
        Copy,
        Count
    };

    Instruction(Type type) : m_type(type) {}

    Type m_type = Count;
};