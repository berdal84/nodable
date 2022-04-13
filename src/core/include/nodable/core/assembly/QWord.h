#pragma once

#include <string>
#include <cstring> // for memset
#include "nodable/core/assertions.h"
#include "nodable/core/types.h"
#include "nodable/core/assembly/Register.h"
#include "nodable/core/reflection/reflection"

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
            u8_t      u8;
            u16_t     u16;
            u32_t     u32;
            u64_t     u64;
            i8_t      i8;
            i16_t     i16;
            i32_t     i32;
            i64_t     i64;
            float     f;
            double    d;
            void*     ptr;
            Register  r;
        };

        QWord() { memset(this, 0, sizeof(*this) ); }
        [[nodiscard]] std::string        to_string()const { return QWord::to_string(*this); }
        [[nodiscard]] static std::string to_string(const QWord&);

        template<typename T>
        operator T() { return get<T>(); }
        operator std::string() { return *static_cast<std::string*>(ptr); }

        R_UNION(QWord)
    };
    static_assert(sizeof(QWord) == 64 / 8 );

} // Asm namespace
} // Nodable namespace