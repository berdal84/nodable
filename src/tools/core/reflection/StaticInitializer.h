#pragma once
#include "Invokable.h"
#include "func_type.h"
#include "type.h"
#include "type_register.h"
#include <vector>

namespace tools
{
    template<typename T>
    class InvokableStaticFunction;

    template<typename T>
    struct StaticInitializer
    {
        type* m_type;
        constexpr static bool is_class_v = std::is_class_v<T>;

        explicit StaticInitializer(const char* _name )
        {
            type* type = type::create<T>(_name);
            m_type = type_register::insert_or_merge(type);
        }

        template<typename F>
        StaticInitializer& add_method(F* _function, const char* _name, const char* _alt_name = "" )
        {
            static_assert(is_class_v);
            {
                auto invokable_ = new InvokableStaticFunction<F>(_function, _name);
                m_type->add_static(_name, invokable_);
            }
            if(_alt_name[0] != '\0')
            {
                auto invokable_ = new InvokableStaticFunction<F>(_function, _alt_name);
                m_type->add_static(_alt_name, invokable_ );
            }
            return *this;
        }

        template<typename R, typename C, typename ...Ts>
        StaticInitializer& add_method(R(C::*_function)(Ts...), const char* _name ) // non static
        {
            static_assert(is_class_v);
            using F = R(C::*)(Ts...);
            {
                auto invokable_ = new InvokableMethod<F>(_function, _name);
                m_type->add_method(_name, invokable_);
            }
            return *this;
        }

        template<typename BaseClassT>
        StaticInitializer& extends()
        {
            static_assert(is_class_v);
            static_assert(std::is_base_of_v<BaseClassT, T>);

            type* base_class = const_cast<type*>(type::get<BaseClassT>()); // get or create
            m_type->add_parent( base_class->id() );
            base_class->add_child( m_type->id() );
            return *this;
        }
    };
} // namespace headless

#define CAT_IMPL(a, b) a##b
#define CAT(a, b) CAT_IMPL(a, b)

#define REFLECT_STATIC_INIT\
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
static void auto_static_initializer()
