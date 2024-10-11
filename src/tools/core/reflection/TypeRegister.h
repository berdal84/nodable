#pragma once

#include <string>
#include <unordered_map>
#include <typeindex>

#include "../log.h"

namespace tools
{
    // forward declaration
    class TypeDesc;
    class ClassDesc;

    /**
     * structure to help register types
     */
    struct TypeRegister
    {
        static std::unordered_map<std::type_index, TypeDesc*>& by_index();
        static TypeDesc* get(std::type_index);
        static ClassDesc* get_class(std::type_index);
        static bool      has(const TypeDesc*);
        static bool      has(std::type_index);
        static TypeDesc* insert(TypeDesc*);
        static TypeDesc* merge(TypeDesc* existing, const TypeDesc* other);
        static TypeDesc* insert_or_merge(TypeDesc*);
        static void      log_statistics();

    };
} // namespace headless