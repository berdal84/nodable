#include <nodable/core/reflection/reflection>
#include <stdexcept>   // std::runtime_error
#include <nodable/core/reflection/type.h>


using namespace Nodable;

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

type type::any  = type::get<any_t>();
type type::null = type::get<null_t>();

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
    if(_src == type::any || _dst == type::any ) // We allow cast to unknown type
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

type typeregister::get(size_t _hash)
{
    auto found = by_hash().find(_hash);
    NODABLE_ASSERT_EX(found != by_hash().end(), "type not found!")
    return found->second;
}

std::map<size_t, type>& typeregister::by_hash()
{
    static std::map<size_t, type> meta_type_register_by_typeid;
    return meta_type_register_by_typeid;
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
                type parent_type = typeregister::get(each);
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
    NODABLE_ASSERT_EX(!_type.is_ptr(), "make_ptr only works with non pointer types!")
    type ptr = _type;
    ptr.m_is_pointer = true;
    return ptr;
}

bool typeregister::has(type _type)
{
    return by_hash().find(_type.hash_code()) != by_hash().end();
}

bool typeregister::has(size_t _hash_code)
{
    auto found = by_hash().find(_hash_code);
    return found != by_hash().end();
}

void typeregister::insert(type _type)
{
    // insert if absent from register
    if( !has(_type.hash_code()))
    {
        by_hash().insert({_type.hash_code(), _type});
        return;
    }

    // merge with existing
    type existing = get(_type.hash_code());
    LOG_MESSAGE("reflection", "Merging %s with %s\n", existing.m_compiler_name.c_str(), _type.m_compiler_name.c_str())
    if( _type.m_name.empty() ) _type.m_name = existing.m_name;
    _type.m_children.insert(existing.m_children.begin(), existing.m_children.end() );
    _type.m_parents.insert(existing.m_parents.begin(), existing.m_parents.end() );

    by_hash().insert_or_assign(_type.hash_code(), _type);
}

void typeregister::log_statistics()
{
    LOG_MESSAGE("R", "Logging reflected types ...\n");

    LOG_MESSAGE("R", "By typeid (%i):\n", by_hash().size() );
    for ( const auto& [type_hash, type] : by_hash() )
    {
        LOG_MESSAGE("R", " %16llu => \"%30s\" (%30s)\n", type_hash, type.m_name.c_str(), type.m_compiler_name.c_str() );
    }

    LOG_MESSAGE("R", "Logging done.\n");
}
