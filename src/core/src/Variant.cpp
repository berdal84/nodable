#include <nodable/Variant.h>
#include <nodable/Log.h> // for LOG_DEBUG(...)
#include <cassert>
#include <nodable/Nodable.h>
#include <nodable/String.h>
#include <nodable/Node.h>

using namespace Nodable;

Variant::Variant()
    : m_is_defined(false)
    , m_type(Reflect::Type_Unknown)
{
}

Variant::~Variant(){};

Reflect::Type Variant::get_type()const
{
	return m_type;
}

bool  Variant::is(Reflect::Type _type)const
{
	return m_type == _type;
}

void Variant::set(double _var)
{
	switch(get_type() ) // TODO: consider using State pattern (a single context with n possible states implementing an interface)
	{
		case Reflect::Type_String:
		{
			m_data.emplace<std::string>(std::to_string(_var) );
			break;
		}

        default:
		{
			m_data.emplace<double>(_var );
			break;
		}
	}
    m_is_defined = true;
}

void Variant::set(const std::string& _var)
{
   set(_var.c_str());
}

void Variant::set(const char* _var)
{
    switch (get_type()) // TODO: consider using State pattern (a single context with n possible states implementing an interface)
    {
        case Reflect::Type_String:
        {
            m_data = _var;
        }

        default:
        {
            m_data = std::string(_var);
        }
    }
    m_is_defined = true;
}

void Variant::set(bool _var)
{
	switch(get_type()) // TODO: consider using State pattern (a single context with n possible states implementing an interface)
	{
		case Reflect::Type_String:
		{
			m_data.emplace<std::string>(_var ? "true" : "false" );
			break;
		}

		case Reflect::Type_Double:
		{
			m_data.emplace<double>(_var ? double(1) : double(0) );
			break;
		}

		default:
		{
			m_data.emplace<bool>(_var );
			break;
		}
	}
    m_is_defined = true;
}

bool Variant::is_defined()const
{
	return m_is_defined;
}

void Variant::undefine()
{
    m_is_defined = false;
}

void Variant::set(const Variant* _other)
{
    set_type(_other->get_type());  // TODO: remove this
	m_data       = _other->m_data;
    m_is_defined = _other->m_is_defined;
    m_type       = _other->m_type;
}

void Variant::set_type(Reflect::Type _type) // TODO: remove this
{
	if (get_type() != _type)
	{
		undefine();
        m_type = _type;

		// Set a default value (this will change the type too)
		switch (_type)
		{
		case Reflect::Type_String:
			m_data.emplace<std::string>();
			break;
		case Reflect::Type_Double:
			m_data.emplace<double>();
			break;
		case Reflect::Type_Boolean:
			m_data.emplace<bool>();
			break;
        case Reflect::Type_Pointer:
			m_data.emplace<void*>();
			break;
		}
	}

}

template<>
[[nodiscard]] i64_t Variant::convert_to<i64_t>()const
{
    switch (get_type())
    {
        case Reflect::Type_String:  return double( mpark::get<std::string>(m_data).size());
        case Reflect::Type_Double:  return mpark::get<double>(m_data);
        case Reflect::Type_Boolean: return mpark::get<bool>(m_data);
        default:           return double(0);
    }
}

template<>
[[nodiscard]] double Variant::convert_to<double>()const
{
    switch (get_type())
    {
        case Reflect::Type_String:  return double( mpark::get<std::string>(m_data).size());
        case Reflect::Type_Double:  return mpark::get<double>(m_data);
        case Reflect::Type_Boolean: return mpark::get<bool>(m_data);
        default:           return double(0);
    }
}

template<>
[[nodiscard]] int Variant::convert_to<int>()const
{
	return (int)this->convert_to<double>();
}

template<>
[[nodiscard]] bool Variant::convert_to<bool>()const
{
    switch (get_type())
    {
        case Reflect::Type_String:  return !mpark::get<std::string>(m_data).empty();
        case Reflect::Type_Double:  return mpark::get<double>(m_data) != 0.0F;
        case Reflect::Type_Boolean: return mpark::get<bool>(m_data);
        default:           return false;
    }
}

template<>
[[nodiscard]] std::string Variant::convert_to<std::string>()const
{
    std::string result;

    switch (get_type())  // TODO: consider using State pattern (a single context with n possible states implementing an interface)
    {
        case Reflect::Type_String:
        {
            result.append( mpark::get<std::string>(m_data) );
            break;
        }

        case Reflect::Type_Double:
        {
            result.append(  String::from(mpark::get<double>(m_data)) );
            break;
        }

        case Reflect::Type_Boolean:
        {
            result.append(mpark::get<bool>(m_data) ? "true" : "false" );
            break;
        }

        case Reflect::Type_Pointer:
        {
            result.append("[&") ;
            result.append( std::to_string( (size_t)mpark::get<void*>(m_data)) );
            result.append("]") ;
            break;
        }

        default:
        {
            result.append("<?>");
            break;
        }
    }

    return result;
}

Variant::operator int()const          { return (int)(double)*this; }
Variant::operator double()const       { return convert_to<double>(); }
Variant::operator bool()const         { return convert_to<bool>(); }
Variant::operator std::string ()const { return convert_to<std::string>(); }

void Variant::define()
{
    /* declare variant as "defined" without affecting a value, existing data will be used */
    m_is_defined = true;
}

