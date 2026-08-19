#pragma once
#include "Invokable.h"
#include "Type_Descriptor.h"
#include "Type_Register.h"

namespace tools
{
    // Forward declarations
    template<typename T> class  Invokable_Static_Function;

    namespace type
    {
        /**
         * To reflect a type statically
         * @tparam T the type to reflect
         */
        template<typename T, bool = std::is_class_v<T> >
        struct Initializer;

        // Default implementation
        template<typename T>
        struct Initializer<T, false>
        {
            static_assert(!std::is_class_v<T>);
            Type_Descriptor *m_type;

            explicit Initializer(const char *_name)
            {
                Type_Descriptor *type = Type_Descriptor::create<T>(_name);
                m_type = Type_Register::insert_or_merge(type);
            }
        };

        // Class implementation
        template<typename T>
        struct Initializer<T, true>
        {
            static_assert(std::is_class_v<T>);
            Class_Descriptor *m_class;

            explicit Initializer(const char *_name)
            {
                Type_Descriptor *type = Class_Descriptor::create<T>(_name);
                m_class = (Class_Descriptor *) Type_Register::insert_or_merge(type);
            }

            template<typename BaseClassT>
            Initializer &extends()
            {
                static_assert(std::is_class_v<BaseClassT>);
                static_assert(std::is_base_of_v<BaseClassT, T>);

                auto base_class = const_cast<Class_Descriptor *>( type::get_class<BaseClassT>()); // get or create
                m_class->add_parent(base_class->id());
                base_class->add_child(m_class->id());
                return *this;
            }
        };
    }
} // namespace headless

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
::tools::type::Initializer<TYPENAME>( NAME_STRING )

#define DEFINE_REFLECT( TYPENAME ) DEFINE_REFLECT_WITH_ALIAS( TYPENAME, #TYPENAME )
