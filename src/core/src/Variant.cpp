#include <nodable/Variant.h>
#include <nodable/Log.h> // for LOG_DEBUG(...)
#include <cassert>
#include <nodable/Nodable.h>
#include <nodable/String.h>
#include <nodable/Node.h>

using namespace Nodable;

Variant::Variant()
    : m_is_defined(false)
    , m_meta_type(nullptr)
{
}

Variant::~Variant(){};

std::shared_ptr<const R::MetaType> Variant::get_meta_type()const
{
	return m_meta_type;
}

bool  Variant::is_meta_type(std::shared_ptr<const R::MetaType> _type)const
{
	return m_meta_type->is(_type);
}

void Variant::set(double _var)
{
	switch(get_meta_type()->get_type() ) // TODO: consider using State pattern (a single context with n possible states implementing an interface)
	{
		case R::Type::String:
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
    switch (get_meta_type()->get_type() ) // TODO: consider using State pattern (a single context with n possible states implementing an interface)
    {
        case R::Type::String:
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
	switch(get_meta_type()->get_type() ) // TODO: consider using State pattern (a single context with n possible states implementing an interface)
	{
		case R::Type::String:
		{
			m_data.emplace<std::string>(_var ? "true" : "false" );
			break;
		}

		case R::Type::Double:
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

void Variant::reset_value()
{
    m_is_defined = false;

    // Set a default value (this will change the type too)
    switch (m_meta_type->get_type())
    {
        case R::Type::String:
            m_data.emplace< R::reflect_value<R::Type::String>::type >();
            break;
        case R::Type::Double:
            m_data.emplace< R::reflect_value<R::Type::Double>::type >();
            break;
        case R::Type::Boolean:
            m_data.emplace< R::reflect_value<R::Type::Boolean>::type >();
            break;
        case R::Type::Class:
            m_data.emplace<Node*>();
            break;
        default:
            break;

    }
}

void Variant::set(const Variant* _other)
{
    set_meta_type(_other->get_meta_type());  // TODO: remove this
	m_data       = _other->m_data;
    m_is_defined = _other->m_is_defined;
    m_meta_type  = _other->m_meta_type;
}

void Variant::set_meta_type(std::shared_ptr<const R::MetaType> _type) // TODO: remove this
{
	if (m_meta_type == nullptr || !m_meta_type->is(_type) )
	{
        m_meta_type = _type;
        reset_value();
	}
}

template<>
[[nodiscard]] i64_t Variant::convert_to<i64_t>()const
{
    switch (get_meta_type()->get_type())
    {
        case R::Type::String:  return double(mpark::get<std::string>(m_data).size());
        case R::Type::Double:  return mpark::get<double>(m_data);
        case R::Type::Boolean: return mpark::get<bool>(m_data);
        default:           return double(0);
    }
}

template<>
[[nodiscard]] double Variant::convert_to<double>()const
{
    switch (get_meta_type()->get_type())
    {
        case R::Type::String:  return double(mpark::get<std::string>(m_data).size());
        case R::Type::Double:  return mpark::get<double>(m_data);
        case R::Type::Boolean: return mpark::get<bool>(m_data);
        default:           return double(0);
    }
}

template<>
[[nodiscard]] Node* Variant::convert_to<Node*>()const
{
    return mpark::get<Node*>(m_data);
}

template<>
int Variant::convert_to<int>()const
{
	return (int)this->convert_to<double>();
}

template<>
bool Variant::convert_to<bool>()const
{
    switch (get_meta_type()->get_type())
    {
        case R::Type::String:  return !mpark::get<std::string>(m_data).empty();
        case R::Type::Double:  return mpark::get<double>(m_data) != 0.0F;
        case R::Type::Boolean: return mpark::get<bool>(m_data);
        default:           return false;
    }
}

template<>
std::string Variant::convert_to<std::string>()const
{
    std::string result;
    switch (get_meta_type()->get_type())  // TODO: consider using State pattern (a single context with n possible states implementing an interface)
    {
        case R::Type::String:
        {
            result.append( mpark::get<std::string>(m_data) );
            break;
        }

        case R::Type::Double:
        {
            result.append(  String::from(mpark::get<double>(m_data)) );
            break;
        }

        case R::Type::Boolean:
        {
            result.append(mpark::get<bool>(m_data) ? "true" : "false" );
            break;
        }

        case R::Type::Class:
        {
            result.append("[&") ;
            result.append( std::to_string( (size_t)mpark::get<Node*>(m_data)) );
            result.append("]") ;
            break;
        }

        default:
        {
            result.append("<?>");
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

