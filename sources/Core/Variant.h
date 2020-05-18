#pragma once

#include "Nodable.h"    // for constants and forward declarations
#include <string>

namespace Nodable{

	enum Type_{
		Type_Unknown,
		Type_Boolean,
		Type_Number,
		Type_String,
		Type_Value,
		Type_Entity,
		Type_Wire,
		Type_COUNT
	};

	/* 
		This class is variant implementation
	*/

	class Variant{
	public:
		Variant();
		~Variant();

		bool        isSet()const;	
		bool        isType(Type_ _type)const;
		void        set(const Variant*);
		void        set(const std::string&);
		void        set(const char*);
		void        set(double);
		void        set(bool);		
		void        setType(Type_ _type);
		Type_       getType()const;
		std::string getTypeAsString  ()const;

		template <typename T>
		T get()const;

		template <>
		double get<double>()const {
			switch (type)
			{
			case Type_String:
			{
				return double((*reinterpret_cast<std::string*>(data)).size());
			}

			case Type_Number:
			{
				return *reinterpret_cast<double*>(data);
			}

			case Type_Boolean:
			{
				return *reinterpret_cast<bool*>(data) ? double(1) : double(0);
			}

			default:
			{
				return double(0);
			}
			}

		}

		template <>
		bool get<bool>()const
		{
			switch (type)
			{
			case Type_String:
			{
				return !(*reinterpret_cast<std::string*>(data)).empty();
			}

			case Type_Number:
			{
				return (*reinterpret_cast<double*>(data)) != 0.0F;
			}

			case Type_Boolean:
			{
				return *reinterpret_cast<bool*>(data);
			}

			default:
			{
				return false;
			}
			}

		}

		template <>
		std::string get<std::string>()const
		{
			switch (type)
			{
			case Type_String:
			{
				return *reinterpret_cast<std::string*>(data);
			}

			case Type_Number:
			{
				// Format the num as a string without any useless ending zeros/dot
				std::string str = std::to_string(*reinterpret_cast<double*>(data));
				str.erase(str.find_last_not_of('0') + 1, std::string::npos);
				if (str.find_last_of('.') + 1 == str.size())
					str.erase(str.find_last_of('.'), std::string::npos);
				return str;
			}

			case Type_Boolean:
			{
				return *reinterpret_cast<bool*>(data) ? "true" : "false";
			}

			default:
			{
				return "NULL";
			}
			}
		}


	private:
		void* data = NULL;
		Type_ type = Type_Unknown;
	};
}