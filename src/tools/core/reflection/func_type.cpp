#include "func_type.h"
#include <algorithm> // find_if
#include "Operator.h"

using namespace tools;

func_type::func_type(std::string _id)
    : m_identifier(_id)
    , m_return_type(type::null())
{
}

void func_type::push_arg(const type* _type, bool _by_reference)
{
   auto index = (u8_t)m_args.size();
   std::string name{"arg_" + std::to_string(index)};
   m_args.emplace_back(index, _type, _by_reference, name );
}

bool func_type::is_exactly(const func_type* _other)const
{
    if ( this == _other )                        return true;
    if ( m_args.size() != _other->m_args.size()) return false;
    if ( m_identifier != _other->m_identifier )  return false;
    if ( m_args.empty() )                        return true;

    size_t i = 0;
    while( i < m_args.size() )
    {
        const type* arg_t       = m_args[i].m_type;
        const type* other_arg_t = _other->m_args[i].m_type;

        if ( !arg_t->equals(other_arg_t) )
        {
            return false;
        }
        i++;
    }
    return true;
}

bool func_type::is_compatible(const func_type* _other)const
{
    if ( this == _other )                        return true;
    if ( m_args.size() != _other->m_args.size()) return false;
    if ( m_identifier != _other->m_identifier )  return false;
    if ( m_args.empty() )                        return true;

    size_t i = 0;
    while( i < m_args.size() )
    {
        const type* arg_t       = m_args[i].m_type;
        const type* other_arg_t = _other->m_args[i].m_type;

        if ( !arg_t->equals(other_arg_t) &&
             !type::is_implicitly_convertible(other_arg_t, arg_t))
        {
            return false;
        }
        i++;
    }
    return true;

}

bool func_type::has_an_arg_of_type(const type* _type) const
{
    auto found = std::find_if( m_args.begin(), m_args.end(), [&_type](const FuncArg& each) { return each.m_type->equals(_type); } );
    return found != m_args.end();
}
