#include "Value.h"
#include "Log.h"		 // for LOG_DBG(...)

using namespace Nodable;

Value::Value()
{
	LOG_DBG("New Value\n");
}

Value::~Value(){};

Type_ Value::getType()const
{
	return this->type;
}

bool  Value::isType(Type_ _type)const
{
	return this->type == _type;
}

void Value::setValue(double _value)
{
	switch(type)
	{
		case Type_String:
		{
			setValue(std::to_string(_value));
			break;
		}

		case Type_Number:
		{
			*(double*)data = _value;
			break;
		}

		default:
		{
			type = Type_Number;
			data = new double(_value);
			break;
		}
	}
	LOG_DBG("Value::setValue(%d)\n", _value);
}

void Value::setValue(std::string _value)
{
	setValue(_value.c_str());
}

void Value::setValue(const char* _value)
{
	switch(type)
	{
		case Type_String:
		{
			*(std::string*)data = _value;
			break;
		}

		case Type_Number:
		{
			*(double*)data = std::stod(_value);
			break;
		}
		
		default:
		{
			data = new std::string(_value);
			type = Type_String;
			break;
		}
	}

	LOG_DBG("Value::setValue(%s)\n", _value);
}

void Value::setValue(bool _value)
{
	switch(type)
	{
		case Type_String:
		{
			*(std::string*)data = _value ? "true" : "false";
			break;
		}

		case Type_Number:
		{
			*(double*)data = _value ? double(1) : double(0);
			break;
		}
		case Type_Boolean:
		{
			*(bool*)data = _value;
			break;

		}
		default:
		{
			data = new bool(_value);
			type = Type_Boolean;
			break;
		}
	}

	LOG_DBG("Value::setValue(%s)\n", _value ? "true" : "false");
}

double Value::getValueAsNumber()const
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

bool Value::getValueAsBoolean()const
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
std::string Value::getValueAsString()const
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

bool Value::isSet()const
{
	return type != Type_Unknown;
}

void Value::setValue(const Value* _v)
{
	type = _v->type;
	data = _v->data;	
}

std::string Value::getTypeAsString()const
{
	switch(type)
	{
		case Type_String:		{return "String";}
		case Type_Number:		{return "Number";}
		case Type_Boolean: 		{return "Boolean";}
		default:				{return "Unknown";}
	}
}