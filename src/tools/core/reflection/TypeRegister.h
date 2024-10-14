#pragma once

#include <string>
#include <unordered_map>
#include <typeindex>

#include "../log.h"

namespace tools
{
    // forward declaration
    class TypeDescriptor;
    class ClassDescriptor;

    /**
     * structure to help register types
     */
    struct TypeRegister
    {
        static std::unordered_map<std::type_index, TypeDescriptor*>& by_index();
        static TypeDescriptor* get(std::type_index);
        static ClassDescriptor* get_class(std::type_index);
        static bool      has(const TypeDescriptor*);
        static bool      has(std::type_index);
        static TypeDescriptor* insert(TypeDescriptor*);
        static TypeDescriptor* merge(TypeDescriptor* existing, const TypeDescriptor* other);
        static TypeDescriptor* insert_or_merge(TypeDescriptor*);
        static void      log_statistics();

    };
} // namespace headless