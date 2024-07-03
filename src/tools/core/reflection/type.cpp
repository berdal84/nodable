#include "type.h"
#include "Invokable.h"
#include "reflection"
#include <stdexcept>// std::runtime_error

using namespace tools;

REFLECT_STATIC_INIT
{
    // declare some types manually to get friendly names

    StaticInitializer<double>("double");
    StaticInitializer<std::string>("string");
    StaticInitializer<bool>("bool");
    StaticInitializer<void>("void");
    StaticInitializer<void*>("void*");
    StaticInitializer< i8_t>("i8");
    StaticInitializer<i16_t>("int");
    StaticInitializer<i32_t>("i32");
    StaticInitializer<i64_t>("i64");
    StaticInitializer<any_t>("any");
    StaticInitializer<null_t>("null");
}

type::type(
    id_t _id,
    id_t _primitive_id,
    const char* _name,
    const char* _compiler_name,
    Flags _flags)
: m_id(_id)
, m_primitive_id(_primitive_id)
, m_name(_name)
, m_compiler_name(_compiler_name)
, m_flags(_flags)
{
}

type::~type()
{
    for (auto* each : m_methods )
        delete each;

    for (auto* each : m_static_methods )
        delete each;
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

bool type::is_implicitly_convertible(const type* _src, const type* _dst )
{
    if( _dst->is_const() )
    {
        return false;
    }
    else if (
        (_src->equals(_dst))
        ||
        (!_src->is_ptr() && !_dst->is_ptr() && _src->m_primitive_id == _dst->m_primitive_id)
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
        //        from                 to
        _src->is<i16_t>() && _dst->is<i32_t>()  ||
        _src->is<i16_t>() && _dst->is<double>() ||
        _src->is<i32_t>() && _dst->is<double>()
    ;
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

bool type::is_child_of(std::type_index _possible_parent_id, bool _selfCheck) const
{
    if (_selfCheck && m_id == _possible_parent_id )
    {
        return true;
    }

    if( !has_parent() )
    {
        return false;
    }

    auto direct_parent_found = m_parents.find( _possible_parent_id );

    // direct parent check
    if ( direct_parent_found != m_parents.end())
    {
        return true;
    }

    // indirect parent check
    for (auto each : m_parents)
    {
        const type* parent_type = type_register::get(each);
        if (parent_type->is_child_of(_possible_parent_id, true))
        {
            return true;
        }
    }

    return false;
};

void type::add_parent(id_t parent)
{
    m_parents.insert(parent);
    m_flags |= Flags_HAS_PARENT;
}

void type::add_child(id_t _child)
{
    m_children.insert( _child );
    m_flags |= Flags_HAS_CHILD;
}

void type::add_static(const std::string& _name, const IInvokable* _invokable)
{
    m_static_methods.insert(_invokable);
    m_static_methods_by_name.insert({_name, _invokable});
}

void type::add_method(const std::string &_name, const IInvokableMethod* _invokable)
{
    m_methods.insert(_invokable);
    m_methods_by_name.insert({_name, _invokable});
}

const IInvokableMethod* type::get_method(const std::string& _name) const
{
    auto found = m_methods_by_name.find(_name);
    if( found != m_methods_by_name.end() )
    {
        return found->second;
    }
    return nullptr;
}

const IInvokable* type::get_static(const std::string& _name)const
{
    auto found = m_static_methods_by_name.find(_name);
    if( found != m_static_methods_by_name.end() )
    {
        return found->second;
    }
    return nullptr;
}

bool type::equals(const type *left, const type *right)
{
    ASSERT(left != nullptr)
    return right != nullptr && left->m_id == right->m_id;
}
