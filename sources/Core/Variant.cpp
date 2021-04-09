#include "Variant.h"
#include "Log.h"		 // for LOG_DEBUG(...)
#include <assert.h>

using namespace Nodable;

Variant::Variant()
{
}

Variant::~Variant(){};

Type Variant::getType()const
{
    Type result;

	size_t i = data.index();

	if ( i != std::variant_npos )
	{
        result = Variant::s_nodableTypeByIndex.at(i);
    }
	else
    {
        result = Type_Any;
    }

	return result;
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

}

bool Variant::isSet()const
{
	return getType() != Type_Any;
}

void Variant::set(const Variant* _other)
{
	data = _other->data;
}

std::string Variant::getTypeAsString()const
{
	switch(getType())
	{
		case Type_String:		{return "String";}
		case Type_Double:		{return "Double";}
		case Type_Boolean: 	{return "Boolean";}
		default:				{return "Unknown";}
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
