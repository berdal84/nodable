#pragma once

#include <string>
#include <map>
#include <type_traits>

#include <nodable/core/Log.h>

namespace Nodable
{
    class type;

    /**
     * structure to help register types
     */
    class typeregister
    {
    public:
        static std::map<size_t, type>& by_hash();
        static type get(size_t _hash);
        static bool has(type);
        static bool has(size_t _hash_code);
        static void insert(type);
        static void log_statistics();
    };
} // namespace Nodable