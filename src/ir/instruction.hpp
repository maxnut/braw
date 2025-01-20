#pragma once

struct Instruction {
    enum Type {
        Move,
        Point,
        Add,
        Subtract,
        Call,
        Count
    };

    Instruction(Type type) : m_type(type) {}

    Type m_type = Count;
};