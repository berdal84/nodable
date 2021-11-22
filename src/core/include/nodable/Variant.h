#pragma once

#include <string>
#include <array>
#include <mpark/variant.hpp> // std::variant implem for C++11

#include <nodable/Nodable.h> // for constants and forward declarations
#include <nodable/Type.h>

namespace Nodable {

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
		mpark::variant<Any, bool, double, std::string> data;
	};
}