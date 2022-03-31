#pragma once

#include <string>
#include "nodable/core/assertions.h"
#include "nodable/core/types.h"
#include "nodable/core/assembly/Register.h"

namespace Nodable
{
namespace assembly
{
    /**
     * @brief 64 bits of data, is union of all types (bool, double, void*, etc.).
     */
    struct QWord
    {
        union {
            bool      b;
            double    d;
            u8_t      u8;
            u64_t     u64;
            void*     ptr;
            Register  r;
            char16_t  words[4];
        };

        QWord() { memset(words, 0, sizeof(words) ); }

        explicit QWord(bool     _value): QWord() { b = _value; }
        explicit QWord(double   _value): QWord() { d = _value; }
        explicit QWord(Register _value): QWord() { r = _value; }
        explicit QWord(u8_t     _value): QWord() { u8 = _value; }
        explicit QWord(u64_t    _value): QWord() { u64 = _value; }
        explicit QWord(void*    _value): QWord() { ptr = _value; }

        explicit operator bool() const        { return b; }
        explicit operator int() const         { return (int)d; }
        explicit operator double() const      { return d; }
        explicit operator u8_t() const        { return u8; }
        explicit operator u64_t() const       { return u64; }
        explicit operator void*() const       { return ptr; }
        explicit operator std::string() const { NODABLE_ASSERT(ptr) return *(std::string*)ptr; }

        std::string to_string()const { return QWord::to_string(*this); }
        static std::string to_string(const QWord&);
    };
    static_assert(sizeof(QWord) == 4 * 2 ); // a word is two char


} // Asm namespace
} // Nodable namespace