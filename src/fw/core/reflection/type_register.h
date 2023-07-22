#pragma once

#include <string>
#include <unordered_map>
#include <typeindex>

#include "../log.h"

namespace fw
{
    // forward declaration
    class type;

    /**
     * structure to help register types
     */
    class type_register
    {
    public:
        static std::unordered_map<std::size_t, type*>& by_index();
        static type* get(std::size_t);
        static bool has(type);
        static bool has(std::size_t);
        static void insert(type*);
        static void log_statistics();
    };
} // namespace headless