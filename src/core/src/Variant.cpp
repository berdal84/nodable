#include <nodable/Variant.h>
#include <nodable/Log.h> // for LOG_DEBUG(...)
#include <assert.h>

using namespace Nodable;

Variant::Variant(): m_isDefined(false)
{
}

Variant::~Variant(){};

Type Variant::getType()const
{
	return Variant::s_nodableTypeByIndex.at(data.index());
}

bool  Variant::isType(Type _type)const
{
	return getType() == _type;
}

void Variant::set(double _var)
{
	switch( getType() )
	{
		case Type_String:
		{
			data.emplace<std::string>( std::to_string(_var) );
			break;
		}

        default:
		{
			data.emplace<double>( _var );
			break;
		}
	}
	m_isDefined = true;
}

void Variant::set(const std::string& _var)
{
   this->set(_var.c_str());
}

void Variant::set(const char* _var)
{
    switch (getType())
    {
        case Type_String:
        {
            data = _var;
        }

        default:
        {
            data = std::string(_var);
        }
    }
    m_isDefined = true;
}

void Variant::set(bool _var)
{
	switch(getType())
	{
		case Type_String:
		{
			data.emplace<std::string>( _var ? "true" : "false" );
			break;
		}

		case Type_Double:
		{
			data.emplace<double>( _var ? double(1) : double(0) );
			break;
		}

		default:
		{
			data.emplace<bool>( _var );
			break;
		}
	}
    m_isDefined = true;
}

bool Variant::isDefined()const
{
	return m_isDefined;
}

void Variant::set(const Variant* _other)
{
	data = _other->data;
    m_isDefined = true;
}

std::string Variant::getTypeAsString()const
{
	switch(getType())
	{
		case Type_String:	{return "string";}
		case Type_Double:	{return "double";}
		case Type_Boolean: 	{return "boolean";}
		default:				{return "unknown";}
	}
}

void Variant::setType(Type _type)
{
	if (getType() != _type)
	{

		// Set a default value (this will change the type too)
		switch (_type)
		{
		case Type_String:
			data.emplace<std::string>();
			break;
		case Type_Double:
			data.emplace<double>();
			break;
		case Type_Boolean:
			data.emplace<bool>();
			break;
		default:
            data.emplace<Any>();
			break;
		}
	}

}

Variant::operator int()const
{
    return (int)(double)*this;
}

Variant::operator double()const
{
	switch (getType())
	{
		case Type_String:  return double( std::get<std::string>(data).size());
		case Type_Double:  return std::get<double>(data);
		case Type_Boolean: return std::get<bool>(data) ? double(1) : double(0);
		default:           return double(0);
	}
}

Variant::operator bool()const {
	switch (getType())
	{
		case Type_String:  return !std::get<std::string>(data).empty();
		case Type_Double:  return std::get<double>(data) != 0.0F;
		case Type_Boolean: return std::get<bool>(data);
		default:           return false;
	}
}

Variant::operator std::string()const {

	switch (getType())
	{
		case Type_String:
		{
			return std::get<std::string>(data);
		}

		case Type_Double:
		{
			// Format the num as a string without any useless ending zeros/dot
			std::string str = std::to_string( std::get<double>(data));
			str.erase(str.find_last_not_of('0') + 1, std::string::npos);
			if (str.find_last_of('.') + 1 == str.size())
				str.erase(str.find_last_of('.'), std::string::npos);
			return str;
		}

		case Type_Boolean:
		{
			return  std::get<bool>(data) ? "true" : "false";
		}

		default:
		{
			return "";
		}
	}
}
