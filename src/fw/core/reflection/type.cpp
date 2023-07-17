#include "reflection"
#include <stdexcept>   // std::runtime_error
#include "type.h"
#include "invokable.h"

using namespace fw;

REGISTER
{
    registration::push<double>("double");
    registration::push<std::string>("string");
    registration::push<bool>("bool");
    registration::push<void>("void");
    registration::push<i16_t>("int");
    registration::push<any_t>("any");
    registration::push<null_t>("null");
}

const type* type::any()
{
    static const type* any  = type::get<any_t>();
    return any;
}

const type* type::null()
{
    static const type* null  = type::get<null_t>();
    return null;
}

bool type::is_ptr(const type* left)
{
    return left->m_is_pointer;
}

bool type::is_implicitly_convertible(const type* _src, const type* _dst )
{
    if( _dst->m_is_const )
    {
        return false;
    }
    else if (
        (_src->equals(_dst))
        ||
        (_src->m_is_pointer && _dst->m_is_pointer && _src->m_primitive_index == _dst->m_primitive_index)
    )
    {
        return true;
    }
    else if(_src->is<any_t>() || _dst->is<any_t>() ) // We allow cast to unknown type
    {
        return true;
    }

    return
        // Allows specific casts:
        //
        _src->m_index == type::get<i16_t>()->m_index &&  // From i16_t
        _dst->m_index == type::get<double>()->m_index;   // To double
}

std::string type::get_fullname() const
{
    std::string result;

    if (m_is_const)
    {
        result.append("const ");
    }

    result.append(m_name);

    if (m_is_pointer)
    {
        result.append("*");
    }

    return result;
}

bool type::any_of(std::vector<const type*> types) const
{
    for ( auto each : types )
    {
        if(equals(each))
        {
            return true;
        }
    }
    return false;
}

bool type::is_ptr()const
{
    return m_is_pointer;
}

bool type::is_child_of(const type* _possible_parent_class, bool _selfCheck) const
{
    bool is_child;

    if (_selfCheck && m_index == _possible_parent_class->m_index )
    {
        is_child = true;
    }
    else if ( m_parents.empty())
    {
        is_child = false;
    }
    else
    {
        auto direct_parent_found = m_parents.find(_possible_parent_class->m_index);

        // direct parent check
        if ( direct_parent_found != m_parents.end())
        {
            is_child = true;
        }
        else // indirect parent check
        {
            bool is_a_parent_is_child_of = false;
            for (auto each : m_parents)
            {
                const type* parent_type = type_register::get(each);
                if (parent_type->is_child_of(_possible_parent_class, true))
                {
                    is_a_parent_is_child_of = true;
                }
            }
            is_child = is_a_parent_is_child_of;
        }
    }
    return is_child;
};

void type::add_parent(hash_code_t _parent)
{
    m_parents.insert(_parent);
}

void type::add_child(hash_code_t _child)
{
    m_children.insert( _child );
}

bool type::is_const() const
{
    return m_is_const;
}

void type::add_static(const std::string& _name, std::shared_ptr<iinvokable> _invokable)
{
    m_static_methods.insert(_invokable);
    m_static_methods_by_name.insert({_name, _invokable});
}

void type::add_method(const std::string &_name, std::shared_ptr<iinvokable_nonstatic> _invokable)
{
    m_methods.insert(_invokable);
    m_methods_by_name.insert({_name, _invokable});
}

std::shared_ptr<iinvokable_nonstatic> type::get_method(const std::string& _name) const
{
    auto found = m_methods_by_name.find(_name);
    if( found != m_methods_by_name.end() )
    {
        return found->second;
    }
    return nullptr;
}

std::shared_ptr<iinvokable> type::get_static(const std::string& _name)const
{
    auto found = m_static_methods_by_name.find(_name);
    if( found != m_static_methods_by_name.end() )
    {
        return found->second;
    }
    return nullptr;
}

