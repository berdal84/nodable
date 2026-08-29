#pragma once
#include "Type_Descriptor.h"
#include "Type_Register.h"

namespace tools
{
    template<typename Type>
    struct Type_Initializer
    {
        Type_Descriptor* type;

        explicit Type_Initializer(const char *_name)
        {
            type = type_register_insert_or_merge( type_create<Type>(_name) );
        }

        template<typename Base_Type>
        Type_Initializer& extends()
        {
            static_assert(std::is_class_v<Base_Type>);
            static_assert(std::is_base_of_v<Base_Type, Type>);

            Type_Descriptor* base_type = type_get<Base_Type>();
            type->class_add_parent(base_type->id);
            base_type->class_add_child(type->id);
            
            return *this;
        }
    };
} // namespace tools

#define CAT_IMPL(a, b) a##b
#define CAT(a, b) CAT_IMPL(a, b)

#define REFLECT_STATIC_INITIALIZER( ... )\
static void auto_static_initializer();\
namespace /* using the same trick as rttr to avoid name conflicts*/\
{\
    struct auto_static_initializer_struct\
    {\
        auto_static_initializer_struct()\
        {\
            auto_static_initializer();\
        }\
    };\
}\
static const auto_static_initializer_struct CAT(auto_static_initializer, __LINE__);\
static void auto_static_initializer()\
{\
    __VA_ARGS__ \
}

#define DEFINE_REFLECT_WITH_ALIAS( TYPENAME, NAME_STRING )\
::tools::Type_Initializer<TYPENAME>( NAME_STRING )

#define DEFINE_REFLECT( TYPENAME ) DEFINE_REFLECT_WITH_ALIAS( TYPENAME, #TYPENAME )
