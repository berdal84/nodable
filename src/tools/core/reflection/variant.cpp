#include "variant.h"
#include "tools/core/format.h"

using namespace tools;

variant::variant()
{}

variant::~variant()
{
    release_mem();
}

variant::variant(
    const variant& other
)
: m_type_id(other.m_type_id)
{
    set(other);
}

template<>
void* variant::to<void*>()const
{
    if(is_defined() && m_type_id == Type_pointer )
    {
        return m_data.ptr;
    }
    return {};
}

template<>
u64_t variant::to<u64_t>()const
{
    if( !is_defined())
        if(m_type_id & (Type_u64 | Type_u32))
            return {};
    return m_data.u64;
}

template<>
double variant::to<double>()const
{
    if( !is_defined() )
    {
        return {};
    }

    switch (m_type_id)
    {
        case Type_string:  return stod(*(std::string*)m_data.ptr);
        case Type_double:  return m_data.d;
        case Type_i16:     return double(m_data.i16);
        case Type_i32:     return double(m_data.i32);
        case Type_bool:    return double(m_data.b);
        default:
            ASSERT(false) // this case is not handled
    }
}

template<>
i16_t variant::to<i16_t>()const
{
    if( !is_defined() )
    {
        return {};
    }

    switch (m_type_id)
    {
        case Type_string:  return (i16_t)stoi(*(std::string*)m_data.ptr);
        case Type_double:  return (i16_t)m_data.d;
        case Type_i16:     return m_data.i16;
        case Type_i32:     return (i16_t)m_data.i32;
        case Type_bool:    return (i16_t)m_data.b;
        default:
            ASSERT(false) // this case is not handled
    }
}

template<>
i32_t variant::to<i32_t>()const
{
    if( !is_defined() )
    {
        return {};
    }

    switch (m_type_id)
    {
        case Type_string:  return stoi(*(std::string*)m_data.ptr );
        case Type_double:  return i16_t(m_data.d);
        case Type_i16:     return m_data.i16;
        case Type_i32:     return m_data.i32;
        case Type_bool:    return i16_t(m_data.b);
        default:
            ASSERT(false) // this case is not handled
    }
}

template<>
bool variant::to<bool>()const
{
    if( !is_defined() )
    {
        return {};
    }

    switch (m_type_id)
    {
        case Type_string: return !((std::string*)m_data.ptr)->empty();
        case Type_double: return (bool)m_data.d;
        case Type_i16:    return (bool)m_data.i16;
        case Type_i32:    return (bool)m_data.i32;
        case Type_bool:   return m_data.b;
        default:
            ASSERT(false) // this case is not handled
    }
}

template<>
std::string variant::to<std::string>()const
{
    if( !is_defined() )
    {
        return "";
    }

    switch (m_type_id)
    {
        case Type_string: return *(std::string*)m_data.ptr;
        case Type_double: return std::to_string(m_data.i16);
        case Type_i16:    return std::to_string(m_data.i32);
        case Type_i32:    return format::number(m_data.d);
        case Type_bool:   return m_data.b ? "true" : "false";
        default:
            // return format::hexadecimal(m_data.u64); // this code was found there, probably a mistake
            ASSERT(false) // this case is not handled
    }
}

void variant::set(const std::string& _value)
{
    set(_value.c_str());
}

void variant::set(void* ptr)
{
    auto* type = type::get<void*>();
    if ( !is_type(type) )
        change_type(type);

    if ( !is_initialized() )
        init_mem();

    m_data.ptr = ptr;
    flag_defined();
}

void variant::set(const char* _value)
{
    auto* type = type::get<std::string>();
    if ( !is_type(type) )
        change_type(type);

    if ( !is_initialized() )
        init_mem();

    auto* str = (std::string*)m_data.ptr;
    str->assign(_value);
    flag_defined();
}

void variant::set(double _value)
{
    auto* type = type::get<double>();
    if ( !is_type(type) )
        change_type(type);

    if ( !is_initialized() )
        init_mem();

    m_data.set<double>(_value);
    flag_defined();
}

void variant::set(i16_t _value)
{
    auto* type = type::get<i16_t>();
    if ( !is_type(type) )
        change_type(type);

    if ( !is_initialized() )
        init_mem();

    m_data.set<i16_t>(_value);
    flag_defined();
}

void variant::set(i32_t _value)
{
    auto* type = type::get<i32_t>();
    if ( !is_type(type) )
        change_type(type);

    if ( !is_initialized() )
        init_mem();

    m_data.set<i32_t>(_value);
    flag_defined();
}

void variant::set(bool _value)
{
    auto* type = type::get<bool>();
    if ( !is_type(type) )
        change_type(type);

    if ( !is_initialized() )
        init_mem();

    m_data.set<bool>(_value);
    flag_defined();
}

void variant::clear_data()
{
    EXPECT( m_flags & Flag_IS_MEM_INITIALIZED, "Variant: cannot reset value, variant not initialized!");

    if ( m_type_id == Type_string)
    {
        ((std::string*)m_data.ptr)->clear();
        return;
    }
    m_data.reset();
}

void variant::init_mem()
{
    ASSERT( (m_flags & Flag_IS_MEM_INITIALIZED) == false );

    if( m_type_id == Type_string )
    {
        // std::string is the only class we handle the instantiation, we use otherwise pointers to allocated memory
        m_data.ptr = new std::string();
    }

    m_flags &= ~Flag_IS_DATA_DEFINED;    // set flag to 0
    m_flags |=  Flag_IS_MEM_INITIALIZED; // set flag to 1
}

void variant::release_mem()
{
    ASSERT( (m_flags & Flag_IS_MEM_INITIALIZED) == true );

    if (m_type_id == Type_string )
    {
        // std::string is the only class we handle the instantiation, we use otherwise pointers to allocated memory
        delete ((std::string*)m_data.ptr);
        m_data.ptr = nullptr;
    }

    m_flags &= ~Flag_IS_DATA_DEFINED;    // set flag to 0
    m_flags &= ~Flag_IS_MEM_INITIALIZED; // set flag to 0
}

void variant::change_type(const type* _type)
{
    if( (m_flags & Flag_ALLOWS_TYPE_CHANGE) == 0)
    {
        ASSERT(m_type->any_of({type::null(), type::any()})); // Only null or any types can change when Flag_ALLOWS_TYPE_CHANGE is OFF.
    }
    m_type = _type; // Enum allows to speed up our switch/case    
    auto* normalized_type = _type->is_ptr() ? type::get<void*>() : _type; // Convert to internal enum:
    ASSERT( normalized_type->equals(m_type) == false ) // It's not a change, use is_type(const type*) first
    m_type_id = type_to_enum(normalized_type);
}

// TODO: should we name this flag_assigned() ?
void variant::flag_defined()
{
    EXPECT(m_type_id != Type_null, "Cannot set defined a variant having a null type.");
    EXPECT(is_initialized(),       "Variant needs to be initialized first.");

    /*
     * Like in c/cpp, a memory space can be initialized (ex: int i;) but not defined by the user.
     * That's why is_defined is just a flag. By switching that flag, user will see the value.
     * Usually this flag is turned on when variant is set.
     */
    m_flags |= Flag_IS_DATA_DEFINED;
}

void variant::set(const variant& _other)
{
    if (_other.m_type_id == Type_null )
        return;

    if ( _other.m_type_id == m_type_id )
    {
        if( !is_initialized() )
            init_mem();
        else
            clear_data();
        m_data = _other.m_data;
        flag_defined();
        return;
    }

    ASSERT(type::is_implicitly_convertible(_other.m_type, m_type));
    switch ( _other.m_type_id )
    {
        Type_bool:        return set(_other.to<bool>() );
        Type_double:      return set(_other.to<double>() );
        Type_i16:         return set(_other.to<i16_t>() );
        Type_i32:         return set(_other.to<i32_t>() );
        Type_string:      return set(_other.to<std::string>());
        default:
            EXPECT(false, "Variant: missing type case for operator=");
    }
}

variant& variant::operator=(const variant &other)
{
    set(other);
    return *this;
}

void variant::set(null_t)
{
    change_type(type::null());
    m_flags &= ~Flag_IS_DATA_DEFINED;
}

// by reference

variant::operator std::string& ()     { ASSERT(is_initialized()) return *((std::string*)m_data.ptr);}
variant::operator bool& ()            { ASSERT(is_initialized())    return m_data.b;}
variant::operator i16_t& ()           { ASSERT(is_initialized())    return m_data.i16;}
variant::operator i32_t& ()           { ASSERT(is_initialized())    return m_data.i32;}
variant::operator u32_t& ()           { ASSERT(is_initialized())    return m_data.u32;}
variant::operator u64_t& ()           { ASSERT(is_initialized())    return m_data.u64;}
variant::operator double& ()          { ASSERT(is_initialized())    return m_data.d;}

// by value

variant::operator std::string() const { ASSERT(is_initialized())    return *((std::string*)m_data.ptr);}
variant::operator bool () const       { ASSERT(is_initialized())    return m_data.b;}
variant::operator i16_t () const      { ASSERT(is_initialized())    return m_data.i16;}
variant::operator i32_t () const      { ASSERT(is_initialized())    return m_data.i32;}
variant::operator u32_t () const      { ASSERT(is_initialized())    return m_data.u32;}
variant::operator u64_t () const      { ASSERT(is_initialized())    return m_data.u64;}
variant::operator double () const     { ASSERT(is_initialized())    return m_data.d;}
variant::operator const char*() const { ASSERT(is_initialized())    return ((std::string*)m_data.ptr)->c_str();}
variant::operator void*() const       { ASSERT(m_type_id == Type_pointer );    return m_data.ptr;}

variant::Type variant::type_to_enum(const tools::type* _type)
{
    if( _type->is<bool>() )        return Type_bool;
    if( _type->is<double>() )      return Type_double;
    if( _type->is<i16_t>() )       return Type_i16;
    if( _type->is<i32_t>() )       return Type_i32;
    if( _type->is<std::string>() ) return Type_string;
    if( _type->is_ptr() )          return Type_pointer;
    if( _type->is<any_t>() )       return Type_any;
    if( _type->is<null_t>() )      return Type_null;
    ASSERT( !_type->is<const char*>() ) // use std::string instead
    ASSERT(false) // Unhandled type;
}

bool variant::is_type(const tools::type* _type) const
{
    return m_type_id == type_to_enum(_type); // compare the internal Type enum values
}
