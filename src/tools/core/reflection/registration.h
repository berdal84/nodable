#pragma once
#include <vector>
#include "type_register.h"
#include "type.h"
#include "func_type.h"
#include "invokable.h"

namespace tools
{
    template<typename T>
    class invokable_static;

    class registration
    {
    public:


        template<typename Type>
        class push
        {
        public:

            type* m_type;

            push(const char* _name )
            {
                type* type = type::create<Type>(_name);
                m_type = type_register::insert_or_merge(type);
            }
        };

        template<typename ClassT>
        class push_class
        {
        public:
            type* m_class;

            explicit push_class(const char* _name )
            {
                m_class = const_cast<type*>(type::get<ClassT>());
                m_class->m_name = _name;
            }

            template<typename F>
            push_class& add_method(F* _function, const char* _name, const char* _alt_name = "" )
            {
                {
                    auto invokable_ = std::make_shared<invokable_static<F>>(_function, _name);
                    m_class->add_static(_name, invokable_);
                }
                if(_alt_name[0] != '\0')
                {
                    auto invokable_ = std::make_shared<invokable_static<F>>(_function, _alt_name);
                    m_class->add_static(_alt_name, invokable_ );
                }
                return *this;
            }

            template<typename R, typename C, typename ...Ts>
            push_class& add_method(R(C::*_function)(Ts...), const char* _name ) // non static
            {
                using F = R(C::*)(Ts...);
                {
                    auto invokable_ = std::make_shared<invokable_nonstatic<F> >(_function, _name);
                    m_class->add_method(_name, invokable_);
                }
                return *this;
            }

            template<typename BaseClassT>
            push_class& extends()
            {
                static_assert(std::is_base_of_v<BaseClassT, ClassT>);

                type* base_class = const_cast<type*>(type::get<BaseClassT>()); // get or create
                m_class->add_parent( base_class->id() );
                base_class->add_child( m_class->id() );
                return *this;
            }

        };
    };
} // namespace headless

#define CAT_IMPL(a, b) a##b
#define CAT(a, b) CAT_IMPL(a, b)

#define REGISTER                                                        \
static void auto_register();                                            \
namespace /* using the same trick as rttr to avoid name conflicts*/     \
{                                                                       \
    struct auto_register_struct                                         \
    {                                                                   \
        auto_register_struct()                                          \
        {                                                               \
            auto_register();                                            \
        }                                                               \
    };                                                                  \
}                                                                       \
static const auto_register_struct CAT(auto_register, __LINE__);         \
static void auto_register()
