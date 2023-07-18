#include "variant.h"
#include "core/format.h"

using namespace fw;

variant::~variant()
{
    ensure_is_initialized(false);
}


template<>
void* variant::convert_to<void*>()const
{
    if( !m_is_defined)
    {
        return nullptr;
    }

    if ( m_type->is_ptr())
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

    if(m_type->is<std::string>() )  return stod(*m_data.ptr_std_string );
    if(m_type->is<double>() )       return m_data.d;
    if(m_type->is<i16_t>() )        return double(m_data.i16);
    if(m_type->is<bool>() )         return double(m_data.b);

    FW_ASSERT(false) // this case is not handled

}

template<>
i16_t variant::convert_to<i16_t>()const
{
    if( !m_is_defined)
    {
        return 0;
    }

    if(m_type->is<std::string>() )  return stoi(*m_data.ptr_std_string );
    if(m_type->is<double>() )       return i16_t(m_data.d);
    if(m_type->is<i16_t>() )        return m_data.i16;
    if(m_type->is<bool>() )         return i16_t(m_data.b);

    FW_ASSERT(false) // this case is not handled
}

template<>
bool variant::convert_to<bool>()const
{
    if( !m_is_defined)
    {
        return false;
    }

    if(m_type->is<std::string>() )  return !m_data.ptr_std_string->empty();
    if(m_type->is<double>() )       return m_data.d != 0.0;
    if(m_type->is<i16_t>() )        return m_data.i16 != 0;
    if(m_type->is<bool>() )         return m_data.b;
    if(m_type->is<void *>() )       return m_data.ptr;
    FW_EXPECT(false,"Case not handled!")
}

template<>
std::string variant::convert_to<std::string>()const
{
    if( !m_is_initialized || !m_is_defined)
    {
        return "";
    }

    if(m_type->is<std::string>() )  return *m_data.ptr_std_string;
    if(m_type->is<i16_t>() )        return std::to_string(m_data.i16);
    if(m_type->is<double>() )       return format::number(m_data.d);
    if(m_type->is<bool>() )         return m_data.b ? "true" : "false";
    if(m_type->is_ptr())            return format::address(m_data.ptr);
    FW_EXPECT(false,"Case not handled!")
}

const type* variant::get_type() const
{
	return m_type;
}

void variant::set(const std::string& _value)
{
    ensure_is_type( type::get<std::string>() );
    ensure_is_initialized(true);
    m_data.ptr_std_string->clear();
    m_data.ptr_std_string->append(_value);
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

    if(m_type->is<double>() )
    {
        m_data.d = 0.0;
    }
    else if(m_type->is<bool>() )
    {
        m_data.b = false;
    }
    else if(m_type->is<i16_t>() )
    {
        m_data.i16 = 0;
    }
    else if(m_type->is<std::string>() )
    {
        m_data.ptr_std_string->clear();
    }
    else if( m_type->is_ptr() )
    {
        m_data.ptr = nullptr;
    }
    else
    {
        FW_EXPECT(false, "Missing case")
    }
}

void variant::ensure_is_initialized(bool _initialize)
{
    if(_initialize == m_is_initialized) return;

    if ( _initialize )
    {
        if(m_type->is<std::string>() )
        {
            m_data.ptr_std_string = new std::string();
        }
    }
    else
    {
        if (m_type->is<std::string>() )
        {
            delete m_data.ptr_std_string;
            m_data.reset();
        }
    }

    m_is_defined     = false; // from external point of view
    m_is_initialized = _initialize;
}

void variant::ensure_is_type(const type* _type)
{
    const type* new_type = normalize_type(_type);

    if( new_type->equals(m_type) )
    {
        return;
    }
    else if( !m_type_change_allowed )
    {
        FW_EXPECT( m_type->any_of({type::null(), type::any()}), "variant's type should not change (or be null or any)" );
    }
    m_type = new_type;
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
     * Like in c/cpp, a memory space can be initialized (ex: int i;) but not defined by the user.
     * That's why is_defined is just a flag. By switching that flag, user will see the value.
     * Usually this flag is turned on when variant is set.
     */
    m_is_defined = _value;
}

const type* variant::normalize_type(const type* _type)
{
    if(_type->is_ptr())
    {
        return type::get<void*>();
    }

    return _type;
}

void variant::set(const variant& _other)
{
    FW_ASSERT( _other.m_type != type::null() )
    FW_ASSERT(type::is_implicitly_convertible(_other.m_type, m_type));

    if(m_type->is<bool>() )
    {
        set( _other.convert_to<bool>() );
    }
    else if(m_type->is<double>() )
    {
        set( _other.convert_to<double>() );
    }
    else if(m_type->is<i16_t>() )
    {
        set( _other.convert_to<i16_t>() );
    }
    else if(m_type->is<std::string>() )
    {
        set( _other.m_data.ptr_std_string->c_str() );
    }
    else if( m_type->is_ptr() )
    {
        set( _other.m_data.ptr );
    }
    else
    {
        FW_EXPECT(false, "Variant: missing type case for operator=");
    }
}

variant::variant(const variant& other)
    : m_type(other.m_type)
    , m_is_initialized(false)
    , m_is_defined(false)
    , m_type_change_allowed(false)
{
    set(other);
}

variant::variant(variant&& other)
{
    // move to this
    ensure_is_type(other.m_type);
    ensure_is_initialized( other.m_is_initialized);
    m_is_defined = other.m_is_defined;
    m_data = other.m_data;

    // clear other
    other.m_data.reset();
    other.m_is_initialized = false;
    other.m_is_defined = false;
}
