#pragma once

#include <unordered_map>
#include <typeindex>

namespace tools
{
    // forward declaration
    class Type_Descriptor;
    class Class_Descriptor;

    /**
     * structure to help register types
     */
    struct Type_Register
    {
        static std::unordered_map<std::type_index, Type_Descriptor*>& by_index();
        static Type_Descriptor* get(std::type_index);
        static Class_Descriptor* get_class(std::type_index);
        static bool      has(const Type_Descriptor*);
        static bool      has(std::type_index);
        static Type_Descriptor* insert(Type_Descriptor*);
        static Type_Descriptor* merge(Type_Descriptor* existing, const Type_Descriptor* other);
        static Type_Descriptor* insert_or_merge(Type_Descriptor*);
        static void      log_statistics();

    };
} // namespace headless