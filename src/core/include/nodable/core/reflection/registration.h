#pragma once
#include <vector>
#include <nodable/core/reflection/type_register.h>
#include <nodable/core/reflection/type.h>
#include <nodable/core/reflection/func_type.h>
#include <nodable/core/reflection/invokable.h>

namespace Nodable
{
    template<typename T>
    class invokable_static;

    class registration
    {
    public:


        template<typename T>
        class push
        {
        public:

            type m_type;

            push(const char* _name )
            {
                m_type = type::create<T>(_name);
                type_register::insert(m_type);
            }
        };

        template<typename T>
        class push_class
        {
        public:
            type              m_class;
            std::vector<type> m_parents;

            push_class(const char* _name )
            {
                m_class = type::create<T>(_name);
            }

            ~push_class()
            {
                type_register::insert(m_class);
                for(auto each_parent : m_parents)
                {
                    type_register::insert(each_parent);
                }
            }

            template<typename F>
            push_class& add_method(F* _function, const char* _name, const char* _alt_name = "" )
            {
                {
                    auto invokable_ = std::make_shared<invokable_static<F>>(_function, _name);
                    m_class.add_static(_name, invokable_);
                }
                if(_alt_name[0] != '\0')
                {
                    auto invokable_ = std::make_shared<invokable_static<F>>(_function, _alt_name);
                    m_class.add_static(_alt_name, invokable_ );
                }
                return *this;
            }

            template<typename R, typename C, typename ...Ts>
            push_class& add_method(R(C::*_function)(Ts...), const char* _name ) // non static
            {
                using F = R(C::*)(Ts...);
                {
                    auto invokable_ = std::make_shared<invokable_nonstatic<F> >(_function, _name);
                    m_class.add_method(_name, invokable_);
                }
                return *this;
            }

            template<typename BASE>
            push_class& extends()
            {
                type parent = type::create<BASE>();
                m_parents.push_back( parent );
                m_class.add_parent( parent.hash_code() );
                parent.add_child( m_class.hash_code() );
                return *this;
            }

        };
    };
} // namespace Nodable

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