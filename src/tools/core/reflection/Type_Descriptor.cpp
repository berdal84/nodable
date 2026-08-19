#include "Type_Descriptor.h"
#include "Invokable.h"
#include "bdc/String_Hash.hpp"
#include <algorithm> // find_if

using namespace tools;

bool type::equals(const Type_Descriptor* left, const Type_Descriptor* right)
{
    ASSERT(left != nullptr);
    return right != nullptr && left->id() == right->id();
}

const Type_Descriptor* type::any()
{
    static const Type_Descriptor* descriptor  = type::get<tools::any>();
    return descriptor;
}

const Type_Descriptor* type::null()
{
    static const Type_Descriptor* descriptor  = type::get<tools::null>();
    return descriptor;
}

bool type::is_implicitly_convertible(const Type_Descriptor* _src, const Type_Descriptor* _dst )
{
    return _src->is_implicitly_convertible(_dst);
}

bool Type_Descriptor::is_implicitly_convertible(const Type_Descriptor* _dst ) const
{
    if( _dst->is_const() )
        return false;

    if (this->equals(_dst))
        return true;

    if (!this->is_ptr() && !_dst->is_ptr() && this->m_primitive_id == _dst->m_primitive_id)
        return true;

    if(this->is<any>() || _dst->is<any>() ) // We allow cast to unknown type
        return true;

    return
        // Allows specific casts:
        //  from                 to
        this->is<i16_t>() && _dst->is<i32_t>()  ||
        this->is<i16_t>() && _dst->is<double>() ||
        this->is<i32_t>() && _dst->is<double>();
}

bool Type_Descriptor::any_of(std::vector<const Type_Descriptor*> types) const
{
    for ( auto each : types )
        if(equals(each))
            return true;
    return false;
}

Class_Descriptor::~Class_Descriptor()
{
    for (auto* each : m_methods )
        delete each;

    for (auto* each : m_static_methods )
        delete each;
}


bool Class_Descriptor::is_child_of(std::type_index _possible_parent_id, bool _selfCheck) const
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
    for (std::type_index parent_id : m_parents)
    {
        auto parent_class = Type_Register::get_class(parent_id);
        if (parent_class->is_child_of(_possible_parent_id, true))
        {
            return true;
        }
    }

    return false;
};

void Class_Descriptor::add_parent(std::type_index parent)
{
    m_parents.insert(parent);
    m_flags |= TypeFlag_HAS_PARENT;
}

void Class_Descriptor::add_child(std::type_index _child)
{
    m_children.insert( _child );
    m_flags |= TypeFlag_HAS_CHILD;
}

void Class_Descriptor::add_static(const bdc::String& method_name, const IInvokable* method_pointer)
{
    m_static_methods.insert(method_pointer);
    bdc::String_Hash string_hash = bdc::string_hash( method_name );
    m_static_methods_by_name.insert({ string_hash.hash, method_pointer});
}

void Class_Descriptor::add_method(const bdc::String& method_name, const IInvokable_Method* method_pointer)
{
    m_methods.insert(method_pointer);
    bdc::String_Hash string_hash = bdc::string_hash( method_name );
    m_methods_by_name.insert({string_hash.hash, method_pointer});
}

const IInvokable_Method* Class_Descriptor::get_method(const bdc::String& method_name) const
{
    bdc::String_Hash string_hash = bdc::string_hash( method_name );
    auto found = m_methods_by_name.find(string_hash.hash);
    if( found != m_methods_by_name.end() )
    {
        return found->second;
    }
    return nullptr;
}

const IInvokable* Class_Descriptor::get_static(const bdc::String& method_name)const
{
    bdc::String_Hash string_hash = bdc::string_hash( method_name );
    auto found = m_static_methods_by_name.find(string_hash.hash);
    if( found != m_static_methods_by_name.end() )
    {
        return found->second;
    }
    return nullptr;
}

void Function_Descriptor::push_arg( const Type_Descriptor* _type, bool _pass_by_ref )
{
    Function_Arg_Descriptor arg{};
    arg.type         = _type;
    arg.pass_by_ref  = _pass_by_ref;
    
    arg.name = bdc::string_printf( "arg_%i", args.size );

    args.push_back(arg);
}

bool Function_Descriptor::is_exactly(const Function_Descriptor* _other)const
{
    if ( this == _other )
        return true;
    if (args.size != _other->args.size)
        return false;
    if ( m_name != _other->m_name )
        return false;
    if ( args.empty() )
        return true;

    size_t i = 0;
    while(i < args.size )
    {
        const Type_Descriptor* arg_t       = args[i].type;
        const Type_Descriptor* other_arg_t = _other->args[i].type;

        if ( !arg_t->equals(other_arg_t) )
        {
            return false;
        }
        i++;
    }
    return true;
}

bool Function_Descriptor::is_compatible(const Function_Descriptor* _other)const
{
    if ( this == _other )
        return true;
    if (args.size != _other->args.size)
        return false;
    if ( m_name != _other->m_name )
        return false;
    if ( args.empty() )
        return true;

    size_t i = 0;
    while(i < args.size )
    {
        const Type_Descriptor* arg_t       = args[i].type;
        const Type_Descriptor* other_arg_t = _other->args[i].type;

        if ( !arg_t->equals(other_arg_t) &&
             !other_arg_t->is_implicitly_convertible(arg_t) )
        {
            return false;
        }
        i++;
    }
    return true;

}

bool Function_Descriptor::has_arg_with_type(const Type_Descriptor* _type) const
{
    auto found = std::find_if(args.begin(), args.end(), [&_type](const Function_Arg_Descriptor& each) { return each.type->equals(_type); } );
    return found != args.end();
}
