#include <fw/core/reflection/reflection>
#include <stdexcept>   // std::runtime_error
#include <fw/core/reflection/type.h>
#include <fw/core/reflection/invokable.h>

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

const type& type::any()
{
    static type any  = type::get<any_t>();
    return any;
}

const type& type::null()
{
    static type null  = type::get<null_t>();
    return null;
}

bool type::is_ptr(type left)
{
    return left.m_is_pointer;
}

bool type::is_ref(type left)
{
    return left.m_is_reference;
}

bool type::is_implicitly_convertible(type _src, type _dst )
{
    if(_src == type::any() || _dst == type::any() ) // We allow cast to unknown type
    {
        return true;
    }
    else if (_src.m_hash_code == _dst.m_hash_code )
    {
        return true;
    }
    else if (is_ptr(_src) && is_ptr(_dst))
    {
        return true;
    }

    auto allow_cast = [&](const type& src, const type& dst)
    {
        return _src.m_hash_code == src.m_hash_code && _dst.m_hash_code == dst.m_hash_code;
    };

    return allow_cast(type::get<i16_t>(), type::get<double>());
}

std::string type::get_fullname() const
{
    std::string result;

    if (is_const())
    {
        result.append("const ");
    }

    result.append(m_name);

    if (is_ptr())
    {
        result.append("*");
    }
    else if (is_ref())
    {
        result.append("&");
    }

    return result;
}

bool type::is_ptr()const
{
    return m_is_pointer;
}

bool type::is_ref()const
{
    return m_is_reference;
}

bool type::is_child_of(type _possible_parent_class, bool _selfCheck) const
{
    bool is_child;

    if (_selfCheck && m_hash_code == _possible_parent_class.m_hash_code )
    {
        is_child = true;
    }
    else if ( m_parents.empty())
    {
        is_child = false;
    }
    else
    {
        auto direct_parent_found = m_parents.find(_possible_parent_class.m_hash_code);

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
                type parent_type = type_register::get(each);
                if (parent_type.is_child_of(_possible_parent_class, true))
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

type type::to_pointer(type _type)
{
    NDBL_EXPECT(!_type.is_ptr(), "make_ptr only works with non pointer types!")
    type ptr = _type;
    ptr.m_is_pointer = true;
    return ptr;
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

std::shared_ptr<iinvokable_nonstatic> type::get_method(const std::string& _name)
{
    auto found = m_methods_by_name.find(_name);
    if( found != m_methods_by_name.end() )
    {
        return found->second;
    }
    return nullptr;
}

std::shared_ptr<iinvokable> type::get_static(const std::string& _name)
{
    auto found = m_static_methods_by_name.find(_name);
    if( found != m_static_methods_by_name.end() )
    {
        return found->second;
    }
    return nullptr;
}

