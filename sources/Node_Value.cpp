#include "Node_Value.h"
#include "Log.h"		 // for LOG_DBG(...)

using namespace Nodable;

Node_Value::Node_Value()
{
	LOG_DBG("New Node_Value\n");
}

Node_Value::~Node_Value(){};

Type_ Node_Value::getType()const
{
	return this->type;
}

bool  Node_Value::isType(Type_ _type)const
{
	return this->type == _type;
}

void Node_Value::setValue(double _value)
{
	switch(type)
	{
		case Type_String:
		{
			s = std::to_string(_value);
			break;
		}

		case Type_Number:
		{
			d = _value;
			break;
		}

		default:
		{
			d = _value;
			type = Type_Number;
			break;
		}
	}
}

void Node_Value::setValue(std::string _value)
{
	setValue(_value.c_str());
}

void Node_Value::setValue(const char* _value)
{
	switch(type)
	{
		case Type_String:
		{
			s = _value;
			break;
		}

		case Type_Number:
		{
			d = std::stod(_value);
			break;
		}
		
		default:
		{
			s = _value;
			type = Type_String;
			break;
		}
	}
}

double Node_Value::getValueAsNumber()const
{
	switch(type)
	{
		case Type_String:
		{
			return (double)s.size();
		}

		case Type_Number:
		{
			return d;
		}

		default:
		{
			return double(0);
		}
	}
	
}

std::string Node_Value::getValueAsString()const
{
	switch(type)
	{
		case Type_String:
		{
			return s;
		}

		case Type_Number:
		{
			// Format the num as a string without any useless ending zeros/dot
			std::string str = std::to_string (d);
			str.erase ( str.find_last_not_of('0') +1, std::string::npos );
			if (str.find_last_of('.') +1 == str.size())
				str.erase ( str.find_last_of('.'), std::string::npos );
			return str;
		}

		default:
		{
			return "NULL";
		}
	}
}

bool Node_Value::isSet()const
{
	return type != Type_Unknown;
}

void Node_Value::setValue(const Node_Value& _v)
{
	type = _v.type;
	s = _v.s;
	d = _v.d;
}

std::string Node_Value::getTypeAsString()const
{
	switch(type)
	{
		case Type_String:
		{
			return "String";
		}

		case Type_Number:
		{
			return "Number";
		}

		default:
		{
			return "Unknown";
		}
	}
}