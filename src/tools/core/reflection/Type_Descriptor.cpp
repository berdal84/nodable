#include "Type_Descriptor.h"
#include "bdc/String_Hash.hpp"
#include <algorithm> // find_if
#include <assert.h>

namespace tools
{

bool type_equals(const Type_Descriptor* a, const Type_Descriptor* b)
{
    assert(a != nullptr);
    return b != nullptr && a->id == b->id;
}

const Type_Descriptor* type_any()
{
    static const Type_Descriptor* descriptor  = type_get<any>();
    return descriptor;
}

const Type_Descriptor* type_null()
{
    static const Type_Descriptor* descriptor  = type_get<null>();
    return descriptor;
}

bool type_is_implicitly_convertible(const Type_Descriptor* src_type, const Type_Descriptor* dest_type )
{
    if( dest_type->is_const() )
        return false;

    if (src_type->equals(dest_type))
        return true;

    if (!src_type->is_ptr() && !dest_type->is_ptr() && src_type->primitive_id == dest_type->primitive_id)
        return true;

    if(src_type->is<any>() || dest_type->is<any>() ) // We allow cast to unknown type
        return true;

    return
        // TODO: use std::is_convertible
        // Allows specific casts:
        //  from                 to
        src_type->is<i16_t>() && dest_type->is<i32_t>()  ||
        src_type->is<i16_t>() && dest_type->is<double>() ||
        src_type->is<i32_t>() && dest_type->is<double>();
}

bool Type_Descriptor::is_implicitly_convertible(const Type_Descriptor* dest_type ) const
{
    return type_is_implicitly_convertible(this, dest_type);
}

bool Type_Descriptor::any_of(std::vector<const Type_Descriptor*> types) const
{
    for ( auto each : types )
        if(equals(each))
            return true;
    return false;
}

bool Type_Descriptor::class_is_child_of(std::type_index _possible_parent_id, bool _selfCheck) const
{
    assert( this->flags & Type_Flags_IS_CLASS );

    if (_selfCheck && id == _possible_parent_id )
    {
        return true;
    }

    if( !has_parent() )
    {
        return false;
    }

    auto direct_parent_found = clss.parents.find( _possible_parent_id );

    // direct parent check
    if ( direct_parent_found != clss.parents.end())
    {
        return true;
    }

    // indirect parent check
    for (std::type_index parent_id : clss.parents)
    {
        const Type_Descriptor* parent_class = type_get(parent_id);
        if (parent_class->class_is_child_of(_possible_parent_id, true))
        {
            return true;
        }
    }

    return false;
};

void Type_Descriptor::class_add_parent(std::type_index parent)
{
    assert( this->flags & Type_Flags_IS_CLASS );;
    clss.parents.insert(parent);
    flags |= Type_Flags_HAS_PARENT;
}

void Type_Descriptor::class_add_child(std::type_index _child)
{
    assert( this->flags & Type_Flags_IS_CLASS );
    clss.children.insert( _child );
    flags |= Type_Flags_HAS_CHILD;
}

void Type_Descriptor::function_push_arg( const Type_Descriptor* _type, bool _pass_by_ref )
{
    assert( this->flags & Type_Flags_IS_FUNCTION );
    Function_Arg_Descriptor arg{};
    arg.type         = _type;
    arg.pass_by_ref  = _pass_by_ref;
    
    arg.name = bdc::string_printf( "arg_%i", function.args.size );

    array_append(this->function.args, arg);
}

bool Type_Descriptor::function_is_exactly(const Type_Descriptor* _other) const
{
    assert( this->flags & Type_Flags_IS_FUNCTION );

    if ( this == _other )
        return true;
    if ( function.args.size != _other->function.args.size)
        return false;
    if ( name != _other->name )
        return false;
    if ( function.args.size == 0 )
        return true;

    size_t i = 0;
    while(i < function.args.size )
    {
        const Type_Descriptor* arg_t       = function.args[i].type;
        const Type_Descriptor* other_arg_t = _other->function.args[i].type;

        if ( !type_equals(arg_t, other_arg_t) )
        {
            return false;
        }
        i++;
    }
    return true;
}

bool Type_Descriptor::function_is_compatible(const Type_Descriptor* _other) const
{
    assert( this->flags & Type_Flags_IS_FUNCTION );

    if ( this == _other )
        return true;
    if ( function.args.size != _other->function.args.size)
        return false;
    if ( name != _other->name )
        return false;
    if ( function.args.size == 0 )
        return true;

    size_t i = 0;
    while(i < function.args.size )
    {
        const Type_Descriptor* arg_t       = function.args[i].type;
        const Type_Descriptor* other_arg_t = _other->function.args[i].type;

        if ( !arg_t->equals(other_arg_t) &&
             !other_arg_t->is_implicitly_convertible(arg_t) )
        {
            return false;
        }
        i++;
    }
    return true;

}

bool Type_Descriptor::function_has_arg_with_type(const Type_Descriptor* _type) const
{
    assert( this->flags & Type_Flags_IS_FUNCTION );

    auto found = std::find_if(function.args.begin(), function.args.end(), [&_type](const Function_Arg_Descriptor& each) { return each.type->equals(_type); } );
    return found != function.args.end();
}

} // namespace tools
