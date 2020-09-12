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

void Variant::set(double _var)
{
	switch(type)
	{
		case Type::String:
		{
			set(std::to_string(_var));
			break;
		}

		case Type::Double:
		{
			*reinterpret_cast<double*>(data) = _var;
			break;
		}

		default:
		{
			type = Type::Double;
			data = new double(_var);
			break;
		}
	}
}

void Variant::set(const std::string& _var)
{
	set(_var.c_str());
}

void Variant::set(const char* _var)
{
	switch(type)
	{
		case Type::String:
		{
			*reinterpret_cast<std::string*>(data) = _var;
			break;
		}

		case Type::Double:
		{
			*reinterpret_cast<double*>(data) = std::stod(_var);
			break;
		}
		
		default:
		{
			data = new std::string(_var);
			type = Type::String;
			break;
		}
	}
}

void Variant::set(bool _var)
{
	switch(type)
	{
		case Type::String:
		{
			*reinterpret_cast<std::string*>(data) = _var ? "true" : "false";
			break;
		}

		case Type::Double:
		{
			*reinterpret_cast<double*>(data) = _var ? double(1) : double(0);
			break;
		}
		case Type::Boolean:
		{
			*reinterpret_cast<bool*>(data) = _var;
			break;

		}
		default:
		{
			data = new bool(_var);
			type = Type::Boolean;
			break;
		}
	}

}

bool Variant::isSet()const
{
	return type != Type::Unknown;
}

void Variant::set(const Variant* _v)
{
	setType(_v->getType());

	switch (type)
	{
	case Type::Boolean:
		set((bool)*_v);
		break;
	case Type::Double:
		set((double)*_v);
		break;
	case Type::String:
		set((std::string)*_v);
		break;
	case Type::Unknown:
		break;
	default:
		NODABLE_ASSERT(false); // The case you're trying to set is not yet implemented
		break;
	}
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
	if (type != _type)
	{
		// Reset data is type has already been initialized
		if (type != Type::Unknown)
		{
			switch (type)
			{
			case Type::Boolean:
				delete reinterpret_cast<bool*>(data);
				break;
			case Type::Double:
				delete reinterpret_cast<double*>(data);
				break;
			case Type::String:
				delete reinterpret_cast<std::string*>(data);
				break;
			default:
				break;
			}

			type = Type::Unknown;
		}

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
	case Type::String:  return double((*reinterpret_cast<std::string*>(data)).size());
	case Type::Double:  return *reinterpret_cast<double*>(data);
	case Type::Boolean: return *reinterpret_cast<bool*>(data) ? double(1) : double(0);
	default:           return double(0);
	}
}

Variant::operator bool()const {
	switch (type) {
	case Type::String:  return !(*reinterpret_cast<std::string*>(data)).empty();
	case Type::Double:  return (*reinterpret_cast<double*>(data)) != 0.0F;
	case Type::Boolean: return *reinterpret_cast<bool*>(data);
	default:           return false;
	}
}

Variant::operator std::string()const {

	switch (type) {
	case Type::String: {
		return *reinterpret_cast<std::string*>(data);
	}

	case Type::Double: {
		// Format the num as a string without any useless ending zeros/dot
		std::string str = std::to_string(*reinterpret_cast<double*>(data));
		str.erase(str.find_last_not_of('0') + 1, std::string::npos);
		if (str.find_last_of('.') + 1 == str.size())
			str.erase(str.find_last_of('.'), std::string::npos);
		return str;
	}

	case Type::Boolean: {
		return *reinterpret_cast<bool*>(data) ? "true" : "false";
	}

	default:
	{
		return "";
	}
	}
}
