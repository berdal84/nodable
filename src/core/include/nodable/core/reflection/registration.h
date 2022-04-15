#pragma once
#include <nodable/core/reflection/typeregister.h>
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
                typeregister::insert(m_type);
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
                typeregister::insert(m_class);
                for(auto each_parent : m_parents)
                {
                    typeregister::insert(each_parent);
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