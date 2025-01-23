#pragma once

#include <iterator>
#include <stdexcept>
#include <stdint.h>

template<typename Iterator>
class Cursor {
public:
    using T = typename std::iterator_traits<Iterator>::value_type;
    Cursor(Iterator begin, Iterator end) : m_current(begin), m_begin(begin), m_end(end) {}

    T& value() {
        return *m_value;
    }

    T& peekNext(uint32_t amount = 1) {
        return next(amount).get().prev(amount).value();
    }

    T& peekPrev(uint32_t amount = 1) {
        return prev(amount).get().next(amount).value();
    }

    Cursor& get() {
        m_value = &(*m_current);
        return *this;
    }

    Cursor& next(uint32_t amount = 1) {
        if(std::next(m_current, amount) > m_end) throw std::out_of_range("Cursor out of range at end");
        m_current += amount;
        return *this;
    }

    Cursor& prev(uint32_t amount = 1) {
        if(std::prev(m_current, amount) < m_begin) throw std::out_of_range("Cursor out of range at begin");
        m_current -= amount;
        return *this;
    }

    void tryNext() {
        if(hasNext())
            next();
    }

    bool hasNext() {
        return m_current != m_end;
    }

    size_t getIndex() {
        return std::distance(m_begin, m_current);
    }

private:
    Iterator m_current;
    Iterator m_begin;
    Iterator m_end;

    T* m_value;
};