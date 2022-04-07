#include <nodable/core/Variant.h>
#include <nodable/core/Log.h> // for LOG_DEBUG(...)
#include <cassert>
#include <nodable/core/types.h>
#include <nodable/core/String.h>
#include <nodable/core/Node.h>

using namespace Nodable;

Variant::~Variant()
{
    if( m_is_initialized)
    {
        set_initialized(false);
    }
}


template<>
void* Variant::convert_to<void*>()const
{
    if( !m_is_defined)
    {
        return nullptr;
    }

    if ( get_meta_type()->is_ptr())
    {
        return m_data.ptr;
    }
    return nullptr;
}

template<>
u64_t Variant::convert_to<u64_t>()const
{
    if( !m_is_defined)
    {
        return u64_t(0);
    }
    return m_data.u64;
}

template<>
double Variant::convert_to<double>()const
{
    if( !m_is_defined)
    {
        return 0.0;
    }

    switch (get_meta_type()->get_type())
    {
        case R::Type::string_t:  return stod((std::string)m_data);
        case R::Type::double_t:  return m_data.d;
        case R::Type::i16_t:     return double(m_data.i16);
        case R::Type::bool_t:    return double(m_data.b);
        default:                 NODABLE_ASSERT(false) // this case is not handled
    }
}

template<>
i16_t Variant::convert_to<i16_t>()const
{
    if( !m_is_defined)
    {
        return 0;
    }

    switch (get_meta_type()->get_type())
    {
        case R::Type::string_t:  return stoi((std::string)m_data);
        case R::Type::double_t:  return i16_t(m_data.d);
        case R::Type::i16_t:     return m_data.i16;
        case R::Type::bool_t:    return i16_t(m_data.b);
        default:                 NODABLE_ASSERT(false) // this case is not handled
    }
}

template<>
bool Variant::convert_to<bool>()const
{
    if( !m_is_defined)
    {
        return false;
    }

    switch (get_meta_type()->get_type())
    {
        case R::Type::string_t:  return !((std::string*)m_data.ptr)->empty();
        case R::Type::double_t:  return m_data.d != 0.0;
        case R::Type::i16_t:     return m_data.i16 != 0;
        case R::Type::bool_t: // pass through
        default:                 return m_data.b;
    }
}

template<>
std::string Variant::convert_to<std::string>()const
{
    if( !m_is_initialized)
    {
        return "uninitialized";
    }

    if(!m_is_defined)
    {
        return "undefined";
    }

    switch (get_meta_type()->get_type())
    {
        case R::Type::string_t: return *(std::string*)m_data.ptr;
        case R::Type::i16_t:    return std::to_string(m_data.i16);
        case R::Type::double_t: return String::fmt_double(m_data.d);
        case R::Type::bool_t:   return m_data.b ? "true" : "false";
        case R::Type::Class:    return String::fmt_ptr(m_data.ptr);
        default:                return "<?>";
    }
}

Variant::MetaType Variant::get_meta_type()const
{
	return m_meta_type;
}

bool  Variant::is_meta_type(MetaType _type)const
{
	return m_meta_type->is_exactly(_type);
}

void Variant::set(double _value)
{
    ensure_is_initialized_as<decltype(_value)>();

    m_data       = _value;
    m_is_defined = true;
}

void Variant::set(i16_t _value)
{
    ensure_is_initialized_as<decltype(_value)>();

    m_data       = _value;
    m_is_defined = true;
}

void Variant::set(const std::string& _value)
{
    ensure_is_initialized_as<decltype(_value)>();

    std::string& ptr = *(std::string*)m_data.ptr;
    ptr.clear();
    ptr.append(_value);

    m_is_defined = true;

}

void Variant::set(const char* _value)
{
    set(std::string{_value});
}

void Variant::set(bool _value)
{
    ensure_is_initialized_as<decltype(_value)>();

    m_data        = _value;
    m_is_defined  = true;
}

bool Variant::is_initialized()const
{
	return m_is_initialized;
}

void Variant::set_initialized(bool _initialize)
{
    auto type = m_meta_type->get_type();

    if ( _initialize )
    {
        NODABLE_ASSERT(m_meta_type)
        // Set a default value (this will change the type too)
        switch ( type )
        {
            case R::Type::string_t: m_data.ptr   = new std::string();
                                    m_is_defined = true;              break;
            case R::Type::double_t: m_data.d     = 0.0;               break;
            case R::Type::i16_t:    m_data.i16   = 0;                 break;
            case R::Type::bool_t:   m_data.b     = false;             break;
            case R::Type::void_t:
            case R::Type::Class:    m_data.ptr   = nullptr;           break;
            default:               break;
        }
    }
    else if ( m_is_defined && m_meta_type->get_type() == R::Type::string_t )
    {
        delete (std::string*)m_data.ptr;
    }

    m_is_initialized = _initialize;
    m_is_defined     = false;
    NODABLE_ASSERT(_initialize == m_is_initialized)
}

void Variant::set(const Variant& _other)
{
    if ( _other.m_meta_type && !_other.m_meta_type->is_exactly(m_meta_type) )
    {
        NODABLE_ASSERT(R::Meta_t::is_implicitly_convertible(_other.m_meta_type, m_meta_type));

        switch(m_meta_type->get_type())
        {
            case R::Type::string_t: set(*(std::string*)_other.m_data.ptr ); break;
            case R::Type::bool_t:   set(_other.convert_to<bool>() ); break;
            case R::Type::double_t: set(_other.convert_to<double>() ); break;
            case R::Type::i16_t:    set(_other.convert_to<i16_t>() ); break;
            case R::Type::void_t:
            case R::Type::Class:   set( _other.m_data.ptr); break;
            default: NODABLE_ASSERT(false) // not handled.
        }
        return;
    }

    switch(m_meta_type->get_type())
    {
        case R::Type::string_t:  set(*(std::string*)_other.m_data.ptr ); break;
        case R::Type::bool_t: set(_other.m_data.b); break;
        case R::Type::double_t:  set(_other.m_data.d); break;
        case R::Type::i16_t:   set( _other.m_data.i16); break;
        case R::Type::void_t:
        case R::Type::Class:   set( _other.m_data.ptr); break;
        default: NODABLE_ASSERT(false) // not handled.
    }
}

void Variant::define_meta_type(MetaType _type)
{
    NODABLE_ASSERT(!m_meta_type); // can't switch from one type to another
    m_meta_type = _type;
    set_initialized(true);
    NODABLE_ASSERT(m_is_initialized);
}

// those operators can't figure in header since they are using templates implem in cpp.
Variant::operator i16_t()const        { NODABLE_ASSERT(m_is_defined) return convert_to<i16_t>(); }
Variant::operator double()const       { NODABLE_ASSERT(m_is_defined) return convert_to<double>(); }
Variant::operator bool()const         { NODABLE_ASSERT(m_is_defined) return convert_to<bool>(); }
Variant::operator std::string ()const { NODABLE_ASSERT(m_is_defined) return convert_to<std::string>(); }
Variant::operator void* ()const       { NODABLE_ASSERT(m_is_defined) return convert_to<void*>(); }

void Variant::force_defined_flag(bool _value )
{
    m_is_defined = _value;
}

assembly::QWord* Variant::get_data_ptr()
{
    if( !m_is_initialized )
    {
        return nullptr;
    }

    return &m_data;
}
