#include "variant.h"
#include "tools/core/format.h"

using namespace tools;

variant::variant()
{}

variant::~variant()
{
    if (m_flags & Flag_OWNS_HEAP_ALLOCATED_MEMORY )
        release_mem();
}

variant::variant(const std::string& val)
: variant(val.c_str())
{}

variant::variant(const char* val)
: m_type(Type_string)
{
    init_mem();
    *(std::string*)m_data.ptr = val;
}

variant::variant(double val)
: m_type(Type_double)
{
    m_data.d = val;
}

variant::variant(i16_t val)
: m_type(Type_i16)
{
    m_data.i16 = val;
}

variant::variant(i32_t val)
: m_type(Type_i32)
{
    m_data.i32 = val;
}

variant::variant(bool val)
: m_type(Type_bool)
{
    m_data.b = val;
}

variant::variant(null val)
: m_type(Type_null)
{
    // set_defined() // when a variant has null type, we consider it as "not defined"
}

variant::variant(const variant& other)
{
    *this = other;
}

template<>
void* variant::to<void*>()const
{
    if( m_type == Type_ptr )
    {
        return m_data.ptr;
    }
    return {};
}

template<>
double variant::to<double>()const
{
    switch (m_type)
    {
        case Type_bool:    return double(m_data.b);
        case Type_double:  return m_data.d;
        case Type_i16:     return double(m_data.i16);
        case Type_i32:     return double(m_data.i32);
        case Type_string:  return stod(*(std::string*)m_data.ptr);
        default:
            ASSERT(false) // this case is not handled
    }
}

template<>
i16_t variant::to<i16_t>()const
{
    switch (m_type)
    {
        case Type_bool:    return (i16_t)m_data.b;
        case Type_double:  return (i16_t)m_data.d;
        case Type_i16:     return m_data.i16;
        case Type_i32:     return (i16_t)m_data.i32;
        case Type_string:  return (i16_t)stoi(*(std::string*)m_data.ptr);
        default:
            ASSERT(false) // this case is not handled
    }
}

template<>
i32_t variant::to<i32_t>()const
{
    switch (m_type)
    {
        case Type_bool:    return i32_t(m_data.b);
        case Type_double:  return i32_t(m_data.d);
        case Type_i16:     return m_data.i16;
        case Type_i32:     return m_data.i32;
        case Type_string:  return stoi(*(std::string*)m_data.ptr );
        default:
            ASSERT(false) // this case is not handled
    }
}

template<>
bool variant::to<bool>()const
{
    switch (m_type)
    {
        case Type_bool:   return m_data.b;
        case Type_double: return (bool)m_data.d;
        case Type_i16:    return (bool)m_data.i16;
        case Type_i32:    return (bool)m_data.i32;
        case Type_string: return !((std::string*)m_data.ptr)->empty();
        default:
            ASSERT(false) // this case is not handled
    }
}

template<>
std::string variant::to<std::string>()const
{
    switch (m_type)
    {
        case Type_bool:   return m_data.b ? "true" : "false";
        case Type_double: return format::number(m_data.d);
        case Type_i16:    return std::to_string(m_data.i16);
        case Type_i32:    return std::to_string(m_data.i32);
        case Type_string: return *(std::string*)m_data.ptr;
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
    if (m_type != Type_ptr)
        change_type(Type_ptr);
    m_data.ptr = ptr;
}

void variant::set(const char* _value)
{
    if ( m_type != Type_string )
        change_type(Type_string);

    if ((m_flags & Flag_OWNS_HEAP_ALLOCATED_MEMORY) == 0 )
        init_mem();

    *(std::string*)m_data.ptr = _value;
}

void variant::set(double _value)
{
    auto* type = type::get<double>();
    if ( m_type != Type_double )
        change_type(Type_double);
    m_data.set<double>(_value);
}

void variant::set(i16_t _value)
{
    if ( m_type != Type_i16 )
        change_type(Type_i16);
    m_data.i16 = _value;
}

void variant::set(i32_t _value)
{
    auto* type = type::get<i32_t>();
    if ( !is_type(type) )
        change_type(type);
    m_data.i32 = _value;
}

void variant::set(bool _value)
{
    auto* type = type::get<bool>();
    if ( !is_type(type) )
        change_type(type);
    m_data.b = _value;
}

void variant::set(null)
{
    change_type(type::null());
}

void variant::set(const variant& _other)
{
    *this = _other;
}

void variant::clear_data()
{
    VERIFY(m_flags & Flag_OWNS_HEAP_ALLOCATED_MEMORY, "Variant: cannot reset value, variant not initialized!");

    if (m_type == Type_string)
    {
        ((std::string*)m_data.ptr)->clear();
        return;
    }
    m_data.reset();
}

void variant::init_mem()
{
    if( m_type == Type_string && ((m_flags & Flag_OWNS_HEAP_ALLOCATED_MEMORY) == 0) )
    {
        // std::string is the only class we handle the instantiation, we use otherwise pointers to allocated memory
        m_data.ptr = new std::string();
    }

    m_flags |=  Flag_OWNS_HEAP_ALLOCATED_MEMORY; // set flag to 1
}

void variant::release_mem()
{
    if ( m_type == Type_string )
    {
        ASSERT(m_flags & Flag_OWNS_HEAP_ALLOCATED_MEMORY );
        // std::string is the only class we handle the instantiation, we use otherwise pointers to allocated memory
        delete ((std::string*)m_data.ptr);
        m_data.ptr = nullptr;
    }
    m_flags &= ~Flag_OWNS_HEAP_ALLOCATED_MEMORY; // set flags to 0
}

void variant::change_type(const TypeDescriptor* _type)
{
    auto* normalized_type = _type->is_ptr() ? type::get<void*>() : _type; // normalize any pointer to void*
    change_type( type_to_enum(normalized_type) );
    init_mem();
}

void variant::change_type(Type new_type)
{
    // Guards when a type is already set (changing type has some rules)
    if(m_type != Type_null )
    {
        ASSERT( ( (m_flags & Flag_ALLOWS_TYPE_CHANGE) == 0) || (m_type == Type_null || m_type == Type_any) ) // Only null or any types can change when Flag_ALLOWS_TYPE_CHANGE is OFF.
    }
    ASSERT(new_type != m_type) // It's not a change, use is_type(const type*) first
    if (m_flags & Flag_OWNS_HEAP_ALLOCATED_MEMORY )
        release_mem();
    m_type = new_type; // Enum allows to speed up our switch/case
}

variant& variant::operator=(const variant &other)
{
    ASSERT(other.m_type != Type_null );

    // copy
    if (other.m_type == m_type )
    {
        if( m_type == Type_string)
            set( *((std::string*)other.m_data.ptr) );
        else
            m_data = other.m_data;
        return *this;
    }

    // cast
    switch ( other.m_type )
    {
        case Type_bool:   this->set(other.to<bool>() ); break;
        case Type_double: this->set(other.to<double>() ); break;
        case Type_i16:    this->set(other.to<i16_t>() ); break;
        case Type_i32:    this->set(other.to<i32_t>() ); break;
        case Type_string: this->set(other.to<std::string>()); break;
        default:
            VERIFY(false, "Variant: missing type case for operator=");
    }
    return *this;
}

// by reference

variant::operator bool& ()            { return m_data.b;}
variant::operator double& ()          { return m_data.d;}
variant::operator i16_t& ()           { return m_data.i16;}
variant::operator i32_t& ()           { return m_data.i32;}
variant::operator std::string& ()     { return *((std::string*)m_data.ptr);}

// by value

variant::operator bool () const       { return m_data.b;}
variant::operator const char*() const { return ((std::string*)m_data.ptr)->c_str();}
variant::operator double () const     { return m_data.d;}
variant::operator i16_t () const      { return m_data.i16;}
variant::operator i32_t () const      { return m_data.i32;}
variant::operator std::string() const { return *((std::string*)m_data.ptr);}
variant::operator void*() const       { return m_data.ptr;}

variant::Type variant::type_to_enum(const tools::TypeDescriptor* _type)
{
    if( _type->is<any>() )       return Type_any;
    if( _type->is<bool>() )        return Type_bool;
    if( _type->is<double>() )      return Type_double;
    if( _type->is<i16_t>() )       return Type_i16;
    if( _type->is<i32_t>() )       return Type_i32;
    if( _type->is<null>() )      return Type_null;
    if( _type->is<std::string>() ) return Type_string;
    if( _type->is_ptr() )          return Type_ptr;
    ASSERT( !_type->is<const char*>() ) // use std::string instead
    ASSERT(false) // Unhandled type;
}

const tools::TypeDescriptor* variant::enum_to_type(Type _type)
{
    switch ( _type )
    {
        case Type_any:     return type::get<any>();
        case Type_bool:    return type::get<bool>();
        case Type_double:  return type::get<double>();
        case Type_i16:     return type::get<i16_t>();
        case Type_i32:     return type::get<i32_t>();
        case Type_null:    return type::get<null>();
        case Type_ptr:     return type::get<void*>();
        case Type_string:  return type::get<std::string>();
        default:
            ASSERT(false) // unhandled type
    }
}

bool variant::is_type(const tools::TypeDescriptor* _type) const
{
    return m_type == type_to_enum(_type); // compare the internal Type enum values
}

bool variant::is_mem_initialized() const
{
    if ( m_type != Type_string ) // only strings are heap allocated
        return true;
    return m_flags & Flag_OWNS_HEAP_ALLOCATED_MEMORY;
}

const TypeDescriptor* variant::get_type() const
{
    return enum_to_type(m_type);
}

const qword *variant::data() const
{
    return &m_data;
}
