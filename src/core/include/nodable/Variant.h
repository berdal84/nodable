#pragma once

#include <string>
#include <array>
#include <memory> // std::shared_ptr
#include <mpark/variant.hpp> // std::variant implem for C++11

#include <nodable/Nodable.h> // for constants and forward declarations
#include <nodable/R.h>

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
		bool is_meta_type(std::shared_ptr<const R::MetaType> _meta_type)const;

        void set(void* _pointer)
        {
            set_meta_type(R::get_meta_type<void *>());
            m_data.emplace<void*>(_pointer);
            m_is_defined = true;
        }

		void set(const Variant*);
		void set(const std::string&);
		void set(const char*);
		void set(double);
		void set(bool);

		void set_meta_type(std::shared_ptr<const R::MetaType> _type);

        template<typename T>
        void set_meta_type()
        {
            set_meta_type(R::get_meta_type<T>());
        };

        std::shared_ptr<const R::MetaType> get_meta_type()const;

        // conversion
        template<typename T>
        T convert_to()const;

		// by reference
		inline operator const void*()const     { return mpark::get<void*>(m_data); }
		inline operator void*()          { return mpark::get<void*>(m_data); }
		inline operator double*()        { return &mpark::get<double>(m_data); }
        inline operator bool*()          { return &mpark::get<bool>(m_data); }
        inline operator std::string* ()  { return &mpark::get<std::string>(m_data); }
        inline operator double&()        { return mpark::get<double>(m_data); }
        inline operator bool&()          { return mpark::get<bool>(m_data); }
        inline operator std::string& ()  { return mpark::get<std::string>(m_data); }

        // by value
        operator int()const;
        operator double()const;
        operator bool()const;
        operator std::string ()const;

    private:
        bool m_is_defined;
        std::shared_ptr<const R::MetaType> m_meta_type;
		mpark::variant<bool, double, std::string, void*> m_data;
    };
}