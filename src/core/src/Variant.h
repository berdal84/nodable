#pragma once

#include "Nodable.h"    // for constants and forward declarations
#include "Type.h"
#include <string>
#include <variant>
#include <array>

namespace Nodable::core{

	/**
		This class is a variant implementation.

	    It wraps std::variant with a Nodable typing
	*/

	class Variant {
	public:
		Variant();
		~Variant();

		bool        isDefined()const;
		bool        isType(Type _type)const;
		void        set(const Variant*);
		void        set(const std::string&);
		void        set(const char*);
		void        set(double);
		void        set(bool);
		void        setType(Type _type);
		Type        getType()const;
		std::string getTypeAsString()const;

        explicit operator int()const;
		explicit operator double()const;
		explicit operator bool()const;
		explicit operator std::string()const;

	private:
        bool m_isDefined;

	    /** Nodable::Type to native type */
	    constexpr static const std::array<Type, 4> s_nodableTypeByIndex = {{
	        Type_Any,
	        Type_Boolean,
	        Type_Double,
	        Type_String
	    }};

        typedef void* Any;
		std::variant<Any, bool, double, std::string> data;
	};
}