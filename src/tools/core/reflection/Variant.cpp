#include "Variant.h"
#include "bdc/String.hpp"
#include "core/Asserts.h"
#include "tools/core/Format.h"

using namespace tools;

Variant::Variant()
{}

Variant::~Variant()
{
    if (m_flags & Flag_OWNS_HEAP_ALLOCATED_MEMORY )
        release_mem();
}

Variant::Variant(const bdc::String& val)
: Variant(val.c_str())
{}

Variant::Variant(const bdc::String val)
: m_type(Type_string)
{
    init_mem();
    *(bdc::String*)m_data.ptr = val;
}

Variant::Variant(double val)
: m_type(Type_double)
{
    m_data.d = val;
}

Variant::Variant(i16_t val)
: m_type(Type_i16)
{
    m_data.i16 = val;
}

Variant::Variant(i32_t val)
: m_type(Type_i32)
{
    m_data.i32 = val;
}

Variant::Variant(bool val)
: m_type(Type_bool)
{
    m_data.b = val;
}

Variant::Variant(null val)
: m_type(Type_null)
{
    // set_defined() // when a Variant has null type, we consider it as "not defined"
}

Variant::Variant(const Variant& other)
{
    *this = other;
}

template<>
void* Variant::to<void*>()const
{
    if( m_type == Type_ptr )
    {
        return m_data.ptr;
    }
    return {};
}

template<>
double Variant::to<double>()const
{
    switch (m_type)
    {
        case Type_bool:    return double(m_data.b);
        case Type_double:  return m_data.d;
        case Type_i16:     return double(m_data.i16);
        case Type_i32:     return double(m_data.i32);
        case Type_string:  return std::stod(((bdc::String*)m_data.ptr)->c_str());
        default:
            ASSERT(false); // this case is not handled
    }
    return {};
}

template<>
i16_t Variant::to<i16_t>()const
{
    switch (m_type)
    {
        case Type_bool:    return (i16_t)m_data.b;
        case Type_double:  return (i16_t)m_data.d;
        case Type_i16:     return m_data.i16;
        case Type_i32:     return (i16_t)m_data.i32;
        case Type_string:  return (i16_t)std::stoi(((bdc::String*)m_data.ptr)->c_str());
        default:
            ASSERT(false); // this case is not handled
    }
    return {};
}

template<>
i32_t Variant::to<i32_t>()const
{
    switch (m_type)
    {
        case Type_bool:    return i32_t(m_data.b);
        case Type_double:  return i32_t(m_data.d);
        case Type_i16:     return m_data.i16;
        case Type_i32:     return m_data.i32;
        case Type_string:  return std::stoi(((bdc::String*)m_data.ptr)->c_str());
        default:
            ASSERT(false); // this case is not handled
    }
    return {};
}

template<>
bool Variant::to<bool>()const
{
    switch (m_type)
    {
        case Type_bool:   return m_data.b;
        case Type_double: return (bool)m_data.d;
        case Type_i16:    return (bool)m_data.i16;
        case Type_i32:    return (bool)m_data.i32;
        case Type_string: return !((bdc::String*)m_data.ptr)->empty();
        default:
            ASSERT(false); // this case is not handled
    }
    return {};
}

template<>
bdc::String Variant::to<bdc::String>()const
{
    switch (m_type)
    {
        case Type_bool:   return m_data.b ? "true" : "false";
        case Type_double: return Format::number(m_data.d);
        case Type_i16:    [[fallthrough]];
        case Type_i32:    return bdc::string_printf("%i", m_data.i32);
        case Type_string: return *(bdc::String*)m_data.ptr;
        default:
            // return Format::hexadecimal(m_data.u64); // this code was found there, probably a mistake
            ASSERT(false); // this case is not handled
            return {};
    }
}

void Variant::set(void* ptr)
{
    if (m_type != Type_ptr)
        change_type(Type_ptr);
    m_data.ptr = ptr;
}

void Variant::set(const bdc::String& _value)
{
    if ( m_type != Type_string )
        change_type(Type_string);

    if ((m_flags & Flag_OWNS_HEAP_ALLOCATED_MEMORY) == 0 )
        init_mem();

    *(bdc::String*)m_data.ptr = _value;
}

void Variant::set(double _value)
{
    if ( m_type != Type_double )
        change_type(Type_double);
    m_data.set<double>(_value);
}

void Variant::set(i16_t _value)
{
    if ( m_type != Type_i16 )
        change_type(Type_i16);
    m_data.i16 = _value;
}

void Variant::set(i32_t _value)
{
    auto* type = type::get<i32_t>();
    if ( !is_type(type) )
        change_type(type);
    m_data.i32 = _value;
}

void Variant::set(bool _value)
{
    auto* type = type::get<bool>();
    if ( !is_type(type) )
        change_type(type);
    m_data.b = _value;
}

void Variant::set(null)
{
    change_type(type::null());
}

void Variant::set(const Variant& _other)
{
    *this = _other;
}

void Variant::clear_data()
{
    VERIFY(m_flags & Flag_OWNS_HEAP_ALLOCATED_MEMORY, "Variant: cannot reset value, Variant not initialized!");

    if (m_type == Type_string)
    {
        bdc::string_release(*(bdc::String*)m_data.ptr);
        return;
    }
    m_data.reset();
}

void Variant::init_mem()
{
    if( m_type == Type_string && ((m_flags & Flag_OWNS_HEAP_ALLOCATED_MEMORY) == 0) )
    {
        // bdc::String is the only class we handle the instantiation, we use otherwise pointers to allocated memory
        m_data.ptr = new bdc::String();
    }

    m_flags |=  Flag_OWNS_HEAP_ALLOCATED_MEMORY; // set flag to 1
}

void Variant::release_mem()
{
    if ( m_type == Type_string )
    {
        ASSERT(m_flags & Flag_OWNS_HEAP_ALLOCATED_MEMORY );
        // bdc::String is the only class we handle the instantiation, we use otherwise pointers to allocated memory
        delete ((bdc::String*)m_data.ptr);
        m_data.ptr = nullptr;
    }
    m_flags &= ~Flag_OWNS_HEAP_ALLOCATED_MEMORY; // set flags to 0
}

void Variant::change_type(const Type_Descriptor* _type)
{
    auto* normalized_type = _type->is_ptr() ? type::get<void*>() : _type; // normalize any pointer to void*
    change_type( type_to_enum(normalized_type) );
    init_mem();
}

void Variant::change_type(Type new_type)
{
    // Guards when a type is already set (changing type has some rules)
    if(m_type != Type_null )
    {
        ASSERT( ( (m_flags & Flag_ALLOWS_TYPE_CHANGE) == 0) || (m_type == Type_null || m_type == Type_any) ); // Only null or any types can change when Flag_ALLOWS_TYPE_CHANGE is OFF.
    }
    ASSERT(new_type != m_type); // It's not a change, use is_type(const type*) first
    if (m_flags & Flag_OWNS_HEAP_ALLOCATED_MEMORY )
        release_mem();
    m_type = new_type; // Enum allows to speed up our switch/case
}

Variant& Variant::operator=(const Variant &other)
{
    ASSERT(other.m_type != Type_null );

    // copy
    if (other.m_type == m_type )
    {
        if( m_type == Type_string)
            set( *((bdc::String*)other.m_data.ptr) );
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
        case Type_string: this->set(other.to<bdc::String>()); break;
        default:
            VERIFY(false, "Variant: missing type case for operator=");
    }
    return *this;
}

// by reference

Variant::operator bool& ()            { return m_data.b;}
Variant::operator double& ()          { return m_data.d;}
Variant::operator i16_t& ()           { return m_data.i16;}
Variant::operator i32_t& ()           { return m_data.i32;}
Variant::operator bdc::String& ()     { return *((bdc::String*)m_data.ptr);}

// by value

Variant::operator bool () const       { return m_data.b;}
Variant::operator const bdc::String() const { return ((bdc::String*)m_data.ptr)->c_str();}
Variant::operator double () const     { return m_data.d;}
Variant::operator i16_t () const      { return m_data.i16;}
Variant::operator i32_t () const      { return m_data.i32;}
Variant::operator bdc::String() const { return *((bdc::String*)m_data.ptr);}
Variant::operator void*() const       { return m_data.ptr;}

Variant::Type Variant::type_to_enum(const tools::Type_Descriptor* _type)
{
    if( _type->is<any>() )       return Type_any;
    if( _type->is<bool>() )        return Type_bool;
    if( _type->is<double>() )      return Type_double;
    if( _type->is<i16_t>() )       return Type_i16;
    if( _type->is<i32_t>() )       return Type_i32;
    if( _type->is<null>() )      return Type_null;
    if( _type->is<bdc::String>() ) return Type_string;
    if( _type->is_ptr() )          return Type_ptr;
    ASSERT( !_type->is<const bdc::String>() ); // use bdc::String instead
    ASSERT(false); // Unhandled type;
    return {};
}

const tools::Type_Descriptor* Variant::enum_to_type(Type _type)
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
        case Type_string:  return type::get<bdc::String>();
        default:
            ASSERT(false); // unhandled type
    }
    return {};
}

bool Variant::is_type(const tools::Type_Descriptor* _type) const
{
    return m_type == type_to_enum(_type); // compare the internal Type enum values
}

bool Variant::is_mem_initialized() const
{
    if ( m_type != Type_string ) // only strings are heap allocated
        return true;
    return m_flags & Flag_OWNS_HEAP_ALLOCATED_MEMORY;
}

const Type_Descriptor* Variant::get_type() const
{
    return enum_to_type(m_type);
}

const QWord *Variant::data() const
{
    return &m_data;
}
