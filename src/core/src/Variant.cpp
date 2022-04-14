#include <nodable/core/Variant.h>
#include <nodable/core/Log.h> // for LOG_DEBUG(...)
#include <nodable/core/types.h>
#include <nodable/core/String.h>

using namespace Nodable;

Variant::~Variant()
{
    if( m_is_initialized)
    {
        ensure_is_initialized(false);
    }
}


template<>
void* Variant::convert_to<void*>()const
{
    if( !m_is_defined)
    {
        return nullptr;
    }

    if ( m_type.is_ptr())
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

    if( m_type == type::get<std::string>() )  return stod(*static_cast<std::string*>(m_data.ptr) );
    if( m_type == type::get<double>() )       return m_data.d;
    if( m_type == type::get<i16_t>() )        return double(m_data.i16);
    if( m_type == type::get<bool>() )         return double(m_data.b);

    NODABLE_ASSERT(false) // this case is not handled

}

template<>
i16_t Variant::convert_to<i16_t>()const
{
    if( !m_is_defined)
    {
        return 0;
    }

    if( m_type == type::get<std::string>() )  return stoi( *static_cast<std::string*>(m_data.ptr) );
    if( m_type == type::get<double>() )       return i16_t(m_data.d);
    if( m_type == type::get<i16_t>() )        return m_data.i16;
    if( m_type == type::get<bool>() )         return  i16_t(m_data.b);

    NODABLE_ASSERT(false) // this case is not handled
}

template<>
bool Variant::convert_to<bool>()const
{
    if( !m_is_defined)
    {
        return false;
    }

    if( m_type == type::get<std::string>() )  return !(static_cast<std::string*>(m_data.ptr))->empty();
    if( m_type == type::get<double>() )       return m_data.d != 0.0;
    if( m_type == type::get<i16_t>() )        return m_data.i16 != 0;
    if( m_type == type::get<bool>() )         return m_data.b;
    if( m_type == type::get<void*>() )        return m_data.ptr;
    NODABLE_ASSERT_EX(false,"Case not handled!")
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

    if( m_type == type::get<std::string>() ) return *static_cast<std::string*>(m_data.ptr);
    if( m_type == type::get<i16_t>() )        return std::to_string(m_data.i16);
    if( m_type == type::get<double>() )       return String::fmt_double(m_data.d);
    if( m_type == type::get<bool>() )         return m_data.b ? "true" : "false";
    if( m_type.is_ptr())                      return String::fmt_ptr(m_data.ptr);
    NODABLE_ASSERT_EX(false,"Case not handled!")
}

const type& Variant::get_type()const
{
	return m_type;
}

void Variant::set(const std::string& _value)
{
    ensure_is_type( type::get<std::string>() );
    ensure_is_initialized(true);
    auto* string = static_cast<std::string*>(m_data.ptr);
    string->clear();
    string->append(_value);
    m_is_defined = true;
}

void Variant::set(const char* _value)
{
    set(std::string{_value});
}

void Variant::set(double _value)
{
    ensure_is_type(type::get<double>());
    ensure_is_initialized();
    m_data.set<double>(_value);
    m_is_defined = true;
}

void Variant::set(i16_t _value)
{
    ensure_is_type(type::get<i16_t>());
    ensure_is_initialized();
    m_data.set<i16_t>(_value);
    m_is_defined = true;
}

void Variant::set(bool _value)
{
    ensure_is_type(type::get<bool>());
    ensure_is_initialized();
    m_data.set<bool>(_value);
    m_is_defined = true;
}

bool Variant::is_initialized()const
{
	return m_is_initialized;
}

void Variant::ensure_is_initialized(bool _initialize)
{
    if(_initialize == m_is_initialized) return;

    if ( _initialize )
    {
        NODABLE_ASSERT( m_type != type::null )

        if( m_type == type::get<double>() )
        {
            m_data.d = 0.0;
        }
        else if( m_type == type::get<bool>() )
        {
            m_data.b = false;
        }
        else if( m_type == type::get<i16_t>() )
        {
            m_data.i16 = 0;
        }
        else if( m_type == type::get<std::string>() )
        {
            m_data.ptr   = new std::string();
            m_is_defined = true;
        }
        else if( m_type.is_ptr() )
        {
            m_data.ptr   = nullptr;
        }
        else if( m_type != type::any )
        {
            NODABLE_ASSERT_EX(false, "Missing case")
        }

    }
    else
    {
        if (m_type == type::get<std::string>() )
        {
            delete (std::string*)m_data.ptr;
        }
        m_is_defined = false;
    }

    m_is_initialized = _initialize;
    NODABLE_ASSERT(_initialize == m_is_initialized)
}

void Variant::ensure_is_type(type _type)
{
    auto clean = clean_type(_type);
    if( !m_type_change_allowed )
    {
        if( clean == m_type )
        {
            return;
        }
        NODABLE_ASSERT_EX( m_type == type::null || m_type == type::any,
                "Variant: Type should not change, expecting it null or any!" );
    }
    m_type = clean;
}

Variant::operator i16_t()const        { NODABLE_ASSERT(m_is_defined) return convert_to<i16_t>(); }
Variant::operator double()const       { NODABLE_ASSERT(m_is_defined) return convert_to<double>(); }
Variant::operator bool()const         { NODABLE_ASSERT(m_is_defined) return convert_to<bool>(); }
Variant::operator std::string ()const { NODABLE_ASSERT(m_is_defined) return convert_to<std::string>(); }
Variant::operator void* ()const       { NODABLE_ASSERT(m_is_defined) return convert_to<void*>(); }

void Variant::force_defined_flag(bool _value )
{
    m_is_defined = _value;
}

type Variant::clean_type(const type& _type)
{
    if(_type.is_ptr())
    {
        return type::get<void*>();
    }
    else if( _type.is_class() ) //---------------------------------- we allow only void* for classes
    {
        if( _type != type::get<std::string>())
        {
            return type::get<void*>();
        }
    }
    return _type;
}


Variant& Variant::operator=(const Variant& _other)
{
    NODABLE_ASSERT( _other.m_type != type::null )
    NODABLE_ASSERT(type::is_implicitly_convertible(_other.m_type, m_type));

    if( m_type == type::get<bool>() )
    {
        set( _other.m_data.b );
    }
    else if( m_type == type::get<double>() )
    {
        set( _other.m_data.d );
    }
    else if( m_type == type::get<i16_t>() )
    {
        set( _other.m_data.i16 );
    }
    else if( m_type == type::get<std::string>() )
    {
        set( static_cast<std::string*>(_other.m_data.ptr) );
    }
    else if( m_type.is_ptr() )
    {
            set( _other.m_data.ptr);
    }
    else
    {
        NODABLE_ASSERT_EX(false, "Variant: missing type case for operator=");
    }
    return *this;
}