#pragma once

#include <string>
#include <cstring> // for memset

#include <nodable/core/assertions.h>
#include <nodable/core/types.h>
#include <nodable/core/reflection/union.h>

namespace ndbl
{
    /**
     * @brief 64 bits of data, is union of all types (bool, double, void*, etc.).
     */
    struct qword
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
        };

        qword() { memset(this, 0, sizeof(*this) ); }
        [[nodiscard]] std::string        to_string()const { return qword::to_string(*this); }
        [[nodiscard]] static std::string to_string(const qword&);

        template<typename T>
        explicit operator T() const { return get<const T>(); }
        explicit operator std::string() const { return *static_cast<std::string*>(ptr); }

        R_UNION(qword)
    };
    static_assert(sizeof(qword) == 64 / 8 );
} // ndbl namespace