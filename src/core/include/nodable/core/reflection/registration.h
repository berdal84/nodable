#pragma once
#include <vector>
#include <nodable/core/reflection/type_register.h>
#include <nodable/core/reflection/type.h>

namespace Nodable
{
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