#include <nodable/core/Variant.h>
#include <nodable/core/Log.h> // for LOG_DEBUG(...)
#include <cassert>
#include <nodable/core/types.h>
#include <nodable/core/Format.h>
#include <nodable/core/Node.h>

using namespace Nodable;

Variant::~Variant()
{
    if( m_is_initialized)
    {
        set_initialized(false);
    }
};

std::shared_ptr<const R::MetaType> Variant::get_meta_type()const
{
	return m_meta_type;
}

bool  Variant::is_meta_type(std::shared_ptr<const R::MetaType> _type)const
{
	return m_meta_type->is(_type);
}

void Variant::set(double _value)
{
    if( !m_is_initialized )
    {
        define_meta_type( R::get_meta_type<double>() );
    }
    NODABLE_ASSERT( is_meta_type( R::get_meta_type<double>() ) )

    m_data.d     = _value;
    m_is_defined = true;
}

void Variant::set(const std::string& _value)
{
    set(_value.c_str());
}

void Variant::set(const char* _value)
{
    if( !m_is_initialized )
    {
        define_meta_type( R::get_meta_type<std::string>() );
    }
    NODABLE_ASSERT( is_meta_type( R::get_meta_type<std::string>() ) )

    auto* ptr = (std::string*)m_data.ptr;
    ptr->clear();
    ptr->append(_value);

    m_is_defined = true;
}

void Variant::set(bool _value)
{
    if( !m_is_initialized )
    {
        define_meta_type( R::get_meta_type<bool>() );
    }
    NODABLE_ASSERT( is_meta_type( R::get_meta_type<bool>() ) )

    m_data.b      = _value;
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
            case R::Type::String:  m_data.ptr = new std::string(); m_is_defined = true; break;
            case R::Type::Double:  m_data.d   = 0;                 break;
            case R::Type::Boolean: m_data.b   = false;             break;
            case R::Type::Void:
            case R::Type::Class:   m_data.ptr = nullptr;           break;
            default:               break;
        }
    }
    else if ( m_is_defined && m_meta_type->get_type() == R::Type::String )
    {
        delete (std::string*)m_data.ptr;
    }

    m_is_initialized = _initialize;
    m_is_defined     = false;
    NODABLE_ASSERT(_initialize == m_is_initialized)
}

void Variant::set(const Variant& _other)
{
    NODABLE_ASSERT(_other.m_meta_type && _other.m_meta_type->is(m_meta_type) ) // do not cast, strict same type required

    switch(m_meta_type->get_type())
    {
        case R::Type::String:  set( ((std::string*)_other.m_data.ptr)->c_str() ); break;
        case R::Type::Boolean: set( _other.m_data.b); break;
        case R::Type::Double:  set( _other.m_data.d); break;
        case R::Type::Void:
        case R::Type::Class:   set( _other.m_data.ptr); break;
        default: NODABLE_ASSERT(false) // not handled.
    }
}

void Variant::define_meta_type(std::shared_ptr<const R::MetaType> _type)
{
    NODABLE_ASSERT(!m_meta_type); // can't switch from one type to another
    m_meta_type = _type;
    set_initialized(true);
    NODABLE_ASSERT(m_is_initialized);
}

template<>
void* Variant::convert_to<void*>()const
{
    if( !m_is_defined)
    {
        return nullptr;
    }

    switch (get_meta_type()->get_type())
    {
        case R::Type::String:
        case R::Type::Double:
        case R::Type::Boolean: return nullptr;
        default:               return (void*)m_data.ptr;
    }
}

template<>
u64_t Variant::convert_to<u64_t>()const
{
    if( !m_is_defined)
    {
        return u64_t(0);
    }
    return (u64_t)m_data;
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
        case R::Type::String:  return stod((std::string)m_data);
        case R::Type::Double:  return m_data.d;
        case R::Type::Boolean: return double(m_data.b);
        default:               NODABLE_ASSERT(false) // this case is not handled
    }
}

template<>
int Variant::convert_to<int>()const
{
	return (int)convert_to<double>();
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
        case R::Type::String:  return !((std::string*)m_data.ptr)->empty();
        case R::Type::Double:  return m_data.d != 0.0F;
        case R::Type::Boolean: // pass through
        default:               return m_data.b;
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
        case R::Type::String:  return *(std::string*)m_data.ptr;
        case R::Type::Double:  return Format::fmt_no_trail(m_data.d);
        case R::Type::Boolean: return m_data.b ? "true" : "false";
        case R::Type::Class:   return Format::fmt_ptr(m_data.ptr);
        default:               return "<?>";
    }
}

// those operators can't figure in header since they are using templates implem in cpp.
Variant::operator int()const          { NODABLE_ASSERT(m_is_defined) return (int)convert_to<double>(); }
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
