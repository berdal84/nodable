#pragma once

#include <string>
#include <cstring> // for memset

#include "fw/core/assertions.h"
#include "fw/core/types.h"
#include "union.h"

namespace fw
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

        qword() { reset(); }
        void reset() { memset(this, 0, sizeof(*this) ); }
        [[nodiscard]] std::string        to_string()const { return qword::to_string(*this); }
        [[nodiscard]] static std::string to_string(const qword&);

        template<typename T, std::enable_if_t<std::is_fundamental_v<T>, bool> = 0>
        explicit operator T() const
        {
            static_assert( std::is_fundamental_v<T>, "Handle only fundamental types");
            return get<const T>();
        }

        template<typename T, std::enable_if_t<!std::is_fundamental_v<T>, bool> = 0>
        explicit operator T() const
        {
            static_assert( !std::is_fundamental_v<T>, "Handle only fundamental types");
            if( ptr == nullptr)
            {
                return {};
            }
            return *(T*)ptr;
        }


        R_UNION(qword)
    };
    static_assert(sizeof(qword) == 64 / 8 );
} // headless namespace