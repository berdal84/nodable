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
        static std::unordered_map<std::type_index, type*>& by_index();
        static type* get(std::type_index);
        static bool  has(type);
        static bool  has(std::type_index);
        static void  insert(type*);
        static type* merge(type* existing, const type* other);
        static type* insert_or_merge(type*);
        static void log_statistics();

    };
} // namespace headless