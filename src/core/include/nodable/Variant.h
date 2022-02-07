#pragma once

#include <string>
#include <array>
#include <mpark/variant.hpp> // std::variant implem for C++11

#include <nodable/Nodable.h> // for constants and forward declarations
#include <nodable/Reflect.h>

namespace Nodable {

    // forward declarations
    class Node;

	/**
		This class is a variant implementation.

	    It wraps std::variant with a Nodable typing
	*/

	class Variant {
	public:
		Variant();
		~Variant();

		bool        isDefined()const;
        void        undefine();
		bool        isType(Reflect::Type _type)const;
		void        set(Node*);
		void        set(const Variant*);
		void        set(const std::string&);
		void        set(const char*);
		void        set(double);
		void        set(bool);
		void        setType(Reflect::Type _type);
        Reflect::Type getType()const;
		std::string getTypeAsString()const;

        // conversion
        template<typename T>
        T convert_to()const;

		// by reference
		inline operator const Node*()const { return mpark::get<Node*>(data); }
		inline operator Node*()        { return mpark::get<Node*>(data); }
		inline operator double*()        { return &mpark::get<double>(data); }
        inline operator bool*()          { return &mpark::get<bool>(data); }
        inline operator std::string* () { return &mpark::get<std::string>(data); }
        inline operator double&()        { return mpark::get<double>(data); }
        inline operator bool&()          { return mpark::get<bool>(data); }
        inline operator std::string& () { return mpark::get<std::string>(data); }

        // by value
        operator int()const;
        operator double()const;
        operator bool()const;
        operator std::string ()const;


        void define();

    private:
        bool m_isDefined;

	    /** Nodable::Type to native type */
	    constexpr static const std::array<Reflect::Type, 5> s_nodableTypeByIndex = {{
            Reflect::Type_Unknown,
            Reflect::Type_Boolean,
            Reflect::Type_Double,
            Reflect::Type_String,
            Reflect::Type_Object_Ptr
	    }};

		mpark::variant<
            std::nullptr_t,
            bool,
            double,
            std::string,
            Node*
		> data;
    };
}