#include "Variant.h"
#include "Log.h"		 // for LOG_DEBUG(...)
#include <assert.h>

using namespace Nodable;

Variant::Variant()
{
}

Variant::~Variant(){};

Type_ Variant::getType()const
{
	return this->type;
}

bool  Variant::isType(Type_ _type)const
{
	return this->type == _type;
}

void Variant::set(double _var)
{
	switch(type)
	{
		case Type_String:
		{
			set(std::to_string(_var));
			break;
		}

		case Type_Number:
		{
			*reinterpret_cast<double*>(data) = _var;
			break;
		}

		default:
		{
			type = Type_Number;
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
		case Type_String:
		{
			*reinterpret_cast<std::string*>(data) = _var;
			break;
		}

		case Type_Number:
		{
			*reinterpret_cast<double*>(data) = std::stod(_var);
			break;
		}
		
		default:
		{
			data = new std::string(_var);
			type = Type_String;
			break;
		}
	}
}

void Variant::set(bool _var)
{
	switch(type)
	{
		case Type_String:
		{
			*reinterpret_cast<std::string*>(data) = _var ? "true" : "false";
			break;
		}

		case Type_Number:
		{
			*reinterpret_cast<double*>(data) = _var ? double(1) : double(0);
			break;
		}
		case Type_Boolean:
		{
			*reinterpret_cast<bool*>(data) = _var;
			break;

		}
		default:
		{
			data = new bool(_var);
			type = Type_Boolean;
			break;
		}
	}

}

bool Variant::isSet()const
{
	return type != Type_Unknown;
}

void Variant::set(const Variant* _v)
{
	setType(_v->getType());

	switch (type)
	{
	case Type_Boolean:
		set((bool)*_v);
		break;
	case Type_Number:
		set((double)*_v);
		break;
	case Type_String:
		set((std::string)*_v);
		break;
	case Type_Unknown:
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
		case Type_String:		{return "String";}
		case Type_Number:		{return "Number";}
		case Type_Boolean: 		{return "Boolean";}
		default:				{return "Unknown";}
	}
}

void Variant::setType(Type_ _type)
{
	if (type != _type)
	{
		// Reset data is type has already been initialized
		if (type != Type_Unknown)
		{
			switch (type)
			{
			case Nodable::Type_Boolean:
				delete reinterpret_cast<bool*>(data);
				break;
			case Nodable::Type_Number:
				delete reinterpret_cast<double*>(data);
				break;
			case Nodable::Type_String:
				delete reinterpret_cast<std::string*>(data);
				break;
			default:
				break;
			}

			type = Type_Unknown;
		}

		// Set a default value (this will change the type too)
		switch (_type)
		{
		case Type_String:
			set("");
			break;
		case Type_Number:
			set(double(0));
			break;
		case Type_Boolean:
			set(false);
			break;
		default:
			break;
		}
	}

}

Variant::operator double()const {
	switch (type) {
	case Type_String:  return double((*reinterpret_cast<std::string*>(data)).size());
	case Type_Number:  return *reinterpret_cast<double*>(data);
	case Type_Boolean: return *reinterpret_cast<bool*>(data) ? double(1) : double(0);
	default:           return double(0);
	}
}

Variant::operator bool()const {
	switch (type) {
	case Type_String:  return !(*reinterpret_cast<std::string*>(data)).empty();
	case Type_Number:  return (*reinterpret_cast<double*>(data)) != 0.0F;
	case Type_Boolean: return *reinterpret_cast<bool*>(data);
	default:           return false;
	}
}

Variant::operator std::string()const {

	switch (type) {
	case Type_String: {
		return *reinterpret_cast<std::string*>(data);
	}

	case Type_Number: {
		// Format the num as a string without any useless ending zeros/dot
		std::string str = std::to_string(*reinterpret_cast<double*>(data));
		str.erase(str.find_last_not_of('0') + 1, std::string::npos);
		if (str.find_last_of('.') + 1 == str.size())
			str.erase(str.find_last_of('.'), std::string::npos);
		return str;
	}

	case Type_Boolean: {
		return *reinterpret_cast<bool*>(data) ? "true" : "false";
	}

	default:
	{
		return "";
	}
	}
}
