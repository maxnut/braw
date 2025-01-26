#pragma once

#include <ostream>

namespace CodeGen {

struct Operand {
    enum Type {
        Register,
        Address,
        Integer,
        Label,
        Count
    };

    Operand(Type t) : m_type(t) {}
    virtual ~Operand() = default;

    Type m_type;

    virtual void emit(std::ostream& os) const = 0;

    friend std::ostream& operator<<(std::ostream& os, const Operand& obj) {
        obj.emit(os);
        return os;
    }
};

}