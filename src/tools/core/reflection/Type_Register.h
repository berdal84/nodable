#pragma once

#include <unordered_map>
#include <typeindex>

namespace tools
{
    // forward declaration
    struct Type_Descriptor;

    /**
     * structure to help register types
     */
    Type_Descriptor*    type_register_get(std::type_index);
    bool                type_register_has(const Type_Descriptor*);
    bool                type_register_has(std::type_index);
    Type_Descriptor*    type_register_insert(Type_Descriptor*);
    Type_Descriptor*    type_register_merge(Type_Descriptor* existing, const Type_Descriptor* other);
    Type_Descriptor*    type_register_insert_or_merge(Type_Descriptor*);
    void                type_register_log_statistics();
    
} // namespace tools