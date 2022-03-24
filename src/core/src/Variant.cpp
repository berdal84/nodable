#include <nodable/core/Variant.h>
#include <nodable/core/Log.h> // for LOG_DEBUG(...)
#include <cassert>
#include <nodable/core/types.h>
#include <nodable/core/String.h>
#include <nodable/core/Node.h>

using namespace Nodable;

Variant::~Variant()
{
    if( m_is_defined)
    {
        set_defined(false);
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
    if( !m_meta_type ) define_meta_type( R::get_meta_type<double>() );
    NODABLE_ASSERT( is_meta_type( R::get_meta_type<double>() ) )

    m_data.m_double = _value;
    m_is_defined    = true;
}

void Variant::set(const std::string& _value)
{
    set(_value.c_str());
}

void Variant::set(const char* _value)
{
    if( !m_meta_type ) define_meta_type( R::get_meta_type<std::string>() );
    NODABLE_ASSERT( is_meta_type( R::get_meta_type<std::string>() ) )

    m_data.m_std_string_ptr->clear();
    m_data.m_std_string_ptr->append(_value);

    m_is_defined = true;
}

void Variant::set(bool _value)
{
    if( !m_meta_type ) define_meta_type( R::get_meta_type<bool>() );
    NODABLE_ASSERT( is_meta_type( R::get_meta_type<bool>() ) )

    m_data.m_bool   = _value;
    m_is_defined    = true;
}

bool Variant::is_defined()const
{
	return m_is_defined;
}

void Variant::set_defined(bool _define)
{
    auto type = m_meta_type->get_type();

    if ( _define )
    {
        // Set a default value (this will change the type too)
        switch ( type )
        {
            case R::Type::String:  m_data.m_std_string_ptr   = new std::string(); m_needs_to_delete_std_string = true; break;
            case R::Type::Double:  m_data.m_double           = 0;                 break;
            case R::Type::Boolean: m_data.m_bool             = false;             break;
            case R::Type::Void:
            case R::Type::Class:   m_data.m_void_ptr         = nullptr;           break;
            default:               break;
        }
        m_is_defined = true;
    }
    else
    {
        if ( m_needs_to_delete_std_string )
        {
            delete m_data.m_std_string_ptr;
            m_data.m_std_string_ptr      = nullptr;
            m_needs_to_delete_std_string = false;
        }
        m_is_defined = false;
    }

    NODABLE_ASSERT(_define == m_is_defined)
}

void Variant::set(const Variant& _other)
{
    NODABLE_ASSERT(_other.m_meta_type && _other.m_meta_type->is(m_meta_type) ) // do not cast, strict same type required

    switch(m_meta_type->get_type())
    {
        case R::Type::String:  set( *_other.m_data.m_std_string_ptr ); break;
        case R::Type::Boolean: set( _other.m_data.m_bool); break;
        case R::Type::Double:  set( _other.m_data.m_double); break;
        case R::Type::Void:
        case R::Type::Class:   set( _other.m_data.m_void_ptr); break;
        default: NODABLE_ASSERT(false) // not handled.
    }
}

void Variant::define_meta_type(std::shared_ptr<const R::MetaType> _type)
{
    NODABLE_ASSERT(!m_meta_type); // can't switch from one type to another
    m_meta_type = _type;
    set_defined(true);
    NODABLE_ASSERT(m_is_defined);
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
        case R::Type::String:  return nullptr;
        case R::Type::Double:  return nullptr;
        case R::Type::Boolean: return nullptr;
        case R::Type::Class:   // fall through
        default:               return (void*)m_data.m_void_ptr;
    }
}

template<>
u64 Variant::convert_to<u64>()const
{
    if( !m_is_defined)
    {
        return 0;
    }

    switch (get_meta_type()->get_type())
    {
        case R::Type::String:  return m_data.m_std_string_ptr->size();
        case R::Type::Double:  return u64(m_data.m_double);
        case R::Type::Boolean: return u64(m_data.m_bool);
        case R::Type::Class:   // fall through
        default:               return u64(m_data.m_void_ptr);
    }
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
        case R::Type::String:  return double(m_data.m_std_string_ptr->size());
        case R::Type::Double:  return m_data.m_double;
        case R::Type::Boolean: return double(m_data.m_bool);
        case R::Type::Class:
        default:               return double((u64)m_data.m_void_ptr);
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
        case R::Type::String:  return !m_data.m_std_string_ptr->empty();
        case R::Type::Double:  return m_data.m_double != 0.0F;
        case R::Type::Boolean: // pass through
        default:               return m_data.m_bool;
    }
}

template<>
std::string Variant::convert_to<std::string>()const
{
    if( !m_is_defined)
    {
        return "undefined";
    }

    std::string result;
    switch (get_meta_type()->get_type())
    {
        case R::Type::String:
        {
            result.append( *m_data.m_std_string_ptr );
            break;
        }

        case R::Type::Double:
        {
            result.append(  String::from(m_data.m_double) );
            break;
        }

        case R::Type::Boolean:
        {
            result.append(m_data.m_bool ? "true" : "false" );
            break;
        }

        case R::Type::Class:
        {
            result.append( String::address_to_hexadecimal(m_data.m_void_ptr) );
            break;
        }

        default:
        {
            result.append("<?>");
        }
    }

    return result;
}

// those operators can't figure in header since they are using templates implem in cpp.
Variant::operator int()const          { return (int)convert_to<double>(); }
Variant::operator double()const       { return convert_to<double>(); }
Variant::operator bool()const         { return convert_to<bool>(); }
Variant::operator std::string ()const { return convert_to<std::string>(); }
Variant::operator void* ()const       { return convert_to<void*>(); }
