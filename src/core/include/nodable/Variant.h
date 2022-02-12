#pragma once

#include <string>
#include <array>
#include <mpark/variant.hpp> // std::variant implem for C++11

#include <nodable/Nodable.h> // for constants and forward declarations
#include <nodable/Reflect.h>

namespace Nodable
{
    /**
     * @brief This class can hold several types such as: bool, double, std::string, etc.. (see m_data member)
     */
	class Variant {
	public:
		Variant();
		~Variant();

        void define();
        bool is_defined()const;
        void undefine();
		bool is(Reflect::Type _type)const;

        template<class T>
        void set(T* _pointer)
        {
            set_type(Reflect::Type_Pointer); // TODO: remove this
            m_data = (void*)_pointer;
            m_is_defined = true;
        }

		void set(const Variant*);
		void set(const std::string&);
		void set(const char*);
		void set(double);
		void set(bool);

		void set_type(Reflect::Type _type);

        template<typename T>
        void set_type()
        {
            set_type( Reflect::cpp<T>::reflect_t );
        };

        Reflect::Type get_type()const;

        // conversion
        template<typename T>
        T convert_to()const;

		// by reference
		template<class T>
		inline operator const T*()const { return reinterpret_cast<T*>( mpark::get<void*>(m_data) ); }
        template<class T>
		inline operator T*() { return reinterpret_cast<T*>( mpark::get<void*>(m_data) ); }
		inline operator double*()        { return &mpark::get<double>(m_data); }
        inline operator bool*()          { return &mpark::get<bool>(m_data); }
        inline operator std::string* () { return &mpark::get<std::string>(m_data); }
        inline operator double&()        { return mpark::get<double>(m_data); }
        inline operator bool&()          { return mpark::get<bool>(m_data); }
        inline operator std::string& () { return mpark::get<std::string>(m_data); }

        // by value
        operator int()const;
        operator double()const;
        operator bool()const;
        operator std::string ()const;

    private:
        bool m_is_defined;
        Reflect::Type m_type;
		mpark::variant<bool, double, std::string, void*> m_data;
    };
}