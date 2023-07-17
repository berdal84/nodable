#pragma once

#include <string>
#include <map>
#include <typeindex>

#include "core/log.h"

namespace fw
{
    class type;

    /**
     * structure to help register types
     */
    class type_register
    {
    public:
        static std::map<std::type_index, const type*>& by_index();
        static const type* get(std::type_index);
        static bool has(type);
        static bool has(std::type_index);
        static void insert(type*);
        static void log_statistics();
    };
} // namespace headless