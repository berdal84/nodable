#include "Variant.h"
#include "Log.h"		 // for LOG_DBG(...)
#include <assert.h>

using namespace Nodable;

Variant::Variant()
{
	LOG_DBG("New Variant\n");
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

void Variant::setValue(double _var)
{
	switch(type)
	{
		case Type_String:
		{
			setValue(std::to_string(_var));
			break;
		}

		case Type_Number:
		{
			*(double*)data = _var;
			break;
		}

		default:
		{
			type = Type_Number;
			data = new double(_var);
			break;
		}
	}
	LOG_DBG("Variant::setValue(%d)\n", _var);
}

void Variant::setValue(const std::string& _var)
{
	setValue(_var.c_str());
}

void Variant::setValue(const char* _var)
{
	switch(type)
	{
		case Type_String:
		{
			*(std::string*)data = _var;
			break;
		}

		case Type_Number:
		{
			*(double*)data = std::stod(_var);
			break;
		}
		
		default:
		{
			data = new std::string(_var);
			type = Type_String;
			break;
		}
	}

	LOG_DBG("Variant::setValue(%s)\n", _var);
}

void Variant::setValue(bool _var)
{
	switch(type)
	{
		case Type_String:
		{
			*(std::string*)data = _var ? "true" : "false";
			break;
		}

		case Type_Number:
		{
			*(double*)data = _var ? double(1) : double(0);
			break;
		}
		case Type_Boolean:
		{
			*(bool*)data = _var;
			break;

		}
		default:
		{
			data = new bool(_var);
			type = Type_Boolean;
			break;
		}
	}

	LOG_DBG("Variant::setValue(%s)\n", _var ? "true" : "false");
}

double Variant::getValueAsNumber()const
{
	switch(type)
	{
		case Type_String:
		{
			return (double)(*(std::string*)data).size();
		}

		case Type_Number:
		{
			return *(double*)data;
		}

		case Type_Boolean:
		{
			return *(bool*)data ? double(1) : double(0);
		}

		default:
		{
			return double(0);
		}
	}
	
}

bool Variant::getValueAsBoolean()const
{
	switch(type)
	{
		case Type_String:
		{
			return !(*(std::string*)data).empty();
		}

		case Type_Number:
		{
			return (*(double*)data) != 0.0F;
		}

		case Type_Boolean:
		{
			return *(bool*)data;
		}

		default:
		{
			return false;
		}
	}
	
}
std::string Variant::getValueAsString()const
{
	switch(type)
	{
		case Type_String:
		{
			return *(std::string*)data;
		}

		case Type_Number:
		{
			// Format the num as a string without any useless ending zeros/dot
			std::string str = std::to_string (*(double*)data);
			str.erase ( str.find_last_not_of('0') +1, std::string::npos );
			if (str.find_last_of('.') +1 == str.size())
				str.erase ( str.find_last_of('.'), std::string::npos );
			return str;
		}

		case Type_Boolean:
		{
			return *(bool*)data ? "true" : "false";
		}

		default:
		{
			return "NULL";
		}
	}
}

bool Variant::isSet()const
{
	return type != Type_Unknown;
}

void Variant::setValue(const Variant* _v)
{
	setType(_v->getType());

	switch (type)
	{
	case Type_Boolean:
		setValue(_v->getValueAsBoolean());
		break;
	case Type_Number:
		setValue(_v->getValueAsNumber());
		break;
	case Type_String:
		setValue(_v->getValueAsString());
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
				delete (bool*)data;
				break;
			case Nodable::Type_Number:
				delete (double*)data;
				break;
			case Nodable::Type_String:
				delete (std::string*)data;
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
			setValue("");
			break;
		case Type_Number:
			setValue(double(0));
			break;
		case Type_Boolean:
			setValue(false);
			break;
		default:
			break;
		}
	}

}
