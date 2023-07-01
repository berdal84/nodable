#include <fw/core/reflection/variant.h>
#include <fw/core/log.h>
#include <fw/core/types.h>
#include <fw/core/format.h>

using namespace fw;

variant::~variant()
{
    if( m_is_initialized)
    {
        ensure_is_initialized(false);
    }
}


template<>
void* variant::convert_to<void*>()const
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
u64_t variant::convert_to<u64_t>()const
{
    if( !m_is_defined)
    {
        return u64_t(0);
    }
    return m_data.u64;
}

template<>
double variant::convert_to<double>()const
{
    if( !m_is_defined)
    {
        return 0.0;
    }

    if( m_type == type::get<std::string>() )  return stod(*static_cast<std::string*>(m_data.ptr) );
    if( m_type == type::get<double>() )       return m_data.d;
    if( m_type == type::get<i16_t>() )        return double(m_data.i16);
    if( m_type == type::get<bool>() )         return double(m_data.b);

    FW_ASSERT(false) // this case is not handled

}

template<>
i16_t variant::convert_to<i16_t>()const
{
    if( !m_is_defined)
    {
        return 0;
    }

    if( m_type == type::get<std::string>() )  return stoi( *static_cast<std::string*>(m_data.ptr) );
    if( m_type == type::get<double>() )       return i16_t(m_data.d);
    if( m_type == type::get<i16_t>() )        return m_data.i16;
    if( m_type == type::get<bool>() )         return  i16_t(m_data.b);

    FW_ASSERT(false) // this case is not handled
}

template<>
bool variant::convert_to<bool>()const
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
    FW_EXPECT(false,"Case not handled!")
}

template<>
std::string variant::convert_to<std::string>()const
{
    if( !m_is_initialized)
    {
        return "uninitialized";
    }

    if(!m_is_defined)
    {
        return "undefined";
    }

    if( m_type == type::get<std::string>() )  return *static_cast<std::string*>(m_data.ptr);
    if( m_type == type::get<i16_t>() )        return std::to_string(m_data.i16);
    if( m_type == type::get<double>() )       return format::number(m_data.d);
    if( m_type == type::get<bool>() )         return m_data.b ? "true" : "false";
    if( m_type.is_ptr())                      return format::address(m_data.ptr);
    FW_EXPECT(false,"Case not handled!")
}

const type& variant::get_type()const
{
	return m_type;
}

void variant::set(const std::string& _value)
{
    ensure_is_type( type::get<std::string>() );
    ensure_is_initialized(true);
    auto* string = static_cast<std::string*>(m_data.ptr);
    string->clear();
    string->append(_value);
    flag_defined();
}

void variant::set(const char* _value)
{
    set(std::string{_value});
}

void variant::set(double _value)
{
    ensure_is_type(type::get<double>());
    ensure_is_initialized();
    m_data.set<double>(_value);
    flag_defined();
}

void variant::set(i16_t _value)
{
    ensure_is_type(type::get<i16_t>());
    ensure_is_initialized();
    m_data.set<i16_t>(_value);
    flag_defined();
}

void variant::set(bool _value)
{
    ensure_is_type(type::get<bool>());
    ensure_is_initialized();
    m_data.set<bool>(_value);
    flag_defined();
}

bool variant::is_initialized()const
{
	return m_is_initialized;
}

void variant::reset_value()
{
    FW_EXPECT(m_is_initialized, "Variant: cannot reset value, variant not intialized!");

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
        std::string& str = *static_cast<std::string*>(m_data.ptr);
        str.clear();
    }
    else if( m_type.is_ptr() )
    {
        m_data.ptr   = nullptr;
    }
    else
    {
        FW_EXPECT(false, "Missing case")
    }
}

void variant::ensure_is_initialized(bool _initialize)
{
    FW_EXPECT(m_type != type::null(), "Variant: cannot ensure is_initialised(...) because type is null!");

    if(_initialize == m_is_initialized) return;

    if ( _initialize )
    {
        if( m_type == type::get<std::string>() )
        {
            m_data.ptr   = new std::string();
        }
    }
    else
    {
        if (m_type == type::get<std::string>() )
        {
            delete (std::string*)m_data.ptr;
        }
    }

    m_is_defined     = false; // from external point of view
    m_is_initialized = _initialize;
}

void variant::ensure_is_type(type _type)
{
    auto clean = clean_type(_type);
    if( !m_type_change_allowed )
    {
        if( clean == m_type )
        {
            return;
        }
        FW_EXPECT( m_type == type::null() || m_type == type::any(),
                "Variant: Type should not change, expecting it null or any!" );
    }
    m_type = clean;
}

variant::operator i16_t()const        { FW_ASSERT(m_is_defined) return convert_to<i16_t>(); }
variant::operator double()const       { FW_ASSERT(m_is_defined) return convert_to<double>(); }
variant::operator bool()const         { FW_ASSERT(m_is_defined) return convert_to<bool>(); }
variant::operator std::string ()const { FW_ASSERT(m_is_defined) return convert_to<std::string>(); }
variant::operator void* ()const       { FW_ASSERT(m_is_defined) return convert_to<void*>(); }

void variant::flag_defined(bool _value )
{
    FW_EXPECT(m_type != type::null(), "Variant: Unable to ensure variant is defined because its type is null!");
    FW_EXPECT(m_is_initialized, "Variant: Unable to ensure variant is defined because it is not initialized!");

    /*
     * Like is c/cpp, a memory space can be initialized (ex: int i;) but not defined by the user.
     * That's why is_defined is just a flag. By switching that flag, user will see the value.
     * Usually this flag is turned on when variant is set.
     */
    m_is_defined = _value;
}

type variant::clean_type(const type& _type)
{
    if(_type.is_ptr())
    {
        return type::get<void*>();
    }

    return _type;
}


variant& variant::operator=(const variant& _other)
{
    FW_ASSERT( _other.m_type != type::null() )
    FW_ASSERT(type::is_implicitly_convertible(_other.m_type, m_type));

    if( m_type == type::get<bool>() )
    {
        set( _other.convert_to<bool>() );
    }
    else if( m_type == type::get<double>() )
    {
        set( _other.convert_to<double>() );
    }
    else if( m_type == type::get<i16_t>() )
    {
        set( _other.convert_to<i16_t>() );
    }
    else if( m_type == type::get<std::string>() )
    {
        set( *static_cast<std::string*>(_other.m_data.ptr) );
    }
    else if( m_type.is_ptr() )
    {
        set( _other.m_data.ptr);
    }
    else
    {
        FW_EXPECT(false, "Variant: missing type case for operator=");
    }
    return *this;
}