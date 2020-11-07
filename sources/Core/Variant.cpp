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
	return this->type;
}

bool  Variant::isType(Type _type)const
{
	return this->type == _type;
}

void Variant::set(double _value)
{
	data = _value;
}

void Variant::set(const std::string& _var)
{
	data = std::string(_var);
}

void Variant::set(const char* _value)
{
	data = std::string(_value);
}

void Variant::set(bool _value)
{
	data = _value;
}

bool Variant::isSet()const
{
	return data.has_value();
}

void Variant::set(const Variant* _v)
{
    if ( _v->data.type() != data.type() )
    {
        LOG_WARNING(3u, "You're setting a Variant from a different type variant\n");
    }

	setType(_v->getType());
    data = _v->data;
}

std::string Variant::getTypeAsString()const
{
	switch(type)
	{
		case Type::String:		{return "String";}
		case Type::Double:		{return "Double";}
		case Type::Boolean: 	{return "Boolean";}
		default:				{return "Unknown";}
	}
}

void Variant::setType(Type _type)
{
	NODABLE_ASSERT(_type == Type::Any || type != _type); // You try to set again the same type

	{
		// Set a default value (this will change the type too)
		switch (_type)
		{
		case Type::String:
			set("");
			break;
		case Type::Double:
			set(double(0));
			break;
		case Type::Boolean:
			set(false);
			break;
		default:
			break;
		}
	}

}

Variant::operator double()const {
	switch (type) {
    case Type::String:  return std::any_cast<std::string>(data).length();
    case Type::Double:  return std::any_cast<double>(data);
	case Type::Boolean: return std::any_cast<double>(data) ? double(1) : double(0);
	default:           return double(0);
	}
}

Variant::operator bool()const {
	switch (type) {
	    case Type::String:  return std::any_cast<std::string>(data).empty();
	case Type::Double:  return std::any_cast<double>(data) != double(0);
	case Type::Boolean: return std::any_cast<bool>(data);
	default:           return false;
	}
}

Variant::operator std::string()const {

    switch (type)
    {
        case Type::String:
        {
            return std::any_cast<std::string>(data);
        }

        case Type::Double:
        {
            // Format the num as a string without any useless ending zeros/dot
            std::string str = std::to_string(std::any_cast<double>(data));
            str.erase(str.find_last_not_of('0') + 1, std::string::npos);
            if (str.find_last_of('.') + 1 == str.size())
                str.erase(str.find_last_of('.'), std::string::npos);
            return str;
        }

        case Type::Boolean:
        {
            return std::any_cast<bool>(data) ? "true" : "false";
        }

        default:
        {
            return "";
        }
    }
}
