#pragma once

#include <string>
#include <map>
#include <type_traits>

#include "fw/core/Log.h"

namespace fw
{
    class type;

    /**
     * structure to help register types
     */
    class type_register
    {
    public:
        static std::map<size_t, type>& by_hash();
        static type get(size_t _hash);
        static bool has(type);
        static bool has(size_t _hash_code);
        static void insert(type);
        static void log_statistics();
    };
} // namespace headless