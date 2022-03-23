#pragma once

#include <string>
#include <array>
#include <memory> // std::shared_ptr
#include <mpark/variant.hpp> // std::variant implem for C++11

#include <nodable/core/types.h> // for constants and forward declarations
#include <nodable/core/reflection/R.h>
#include <nodable/core/assertions.h>

namespace Nodable
{
    // forward declarations
    class Node;

    /**
     * @brief This class can hold several types such as: bool, double, std::string, etc.. (see m_data member)
     */
	class Variant
    {
	public:
		Variant();
		~Variant();

        bool is_defined()const;
        void set_defined(bool _defined);
		bool is_meta_type(std::shared_ptr<const R::MetaType> _meta_type)const;

        void set(void* _pointer);
		void set(const Variant*);
		void set(const std::string&);
		void set(const char*);
		void set(double);
		void set(bool);

		void define_meta_type(std::shared_ptr<const R::MetaType> _type);

        template<typename T>
        void set_meta_type()
        {
            define_meta_type(R::get_meta_type<T>());
        };

        std::shared_ptr<const R::MetaType> get_meta_type()const;

        template<typename T> T convert_to()const;

        // by reference
        template<typename T> operator const T*()const  { NODABLE_ASSERT(m_is_defined) return reinterpret_cast<const T*>(m_data.m_void_ptr); }
        template<typename T> operator T*()             { NODABLE_ASSERT(m_is_defined) return reinterpret_cast<T*>(m_data.m_void_ptr); }

		operator double*()        { NODABLE_ASSERT(m_is_defined) return &m_data.m_double; }
        operator bool*()          { NODABLE_ASSERT(m_is_defined) return &m_data.m_bool; }
        operator std::string* ()  { NODABLE_ASSERT(m_is_defined) return m_data.m_std_string_ptr; }
        operator void* ()         { NODABLE_ASSERT(m_is_defined) return m_data.m_void_ptr; }
        operator double&()        { NODABLE_ASSERT(m_is_defined) return m_data.m_double; }
        operator bool&()          { NODABLE_ASSERT(m_is_defined) return m_data.m_bool; }
        operator std::string& ()  { NODABLE_ASSERT(m_is_defined) return *m_data.m_std_string_ptr; }

        // by value
        operator int()const;
        operator double()const;
        operator bool()const;
        operator std::string ()const;
        operator void* ()const;

    private:
        bool m_is_defined;
        std::shared_ptr<const R::MetaType> m_meta_type;
		union DataType
		{
		    DataType(){ m_size_t = 0; }
		    bool           m_bool;
		    //i8             m_i8;           // not handled yet
		    //i16            m_i16;                   //
		    //i32            m_i32;                   //
		    //i64            m_i64;                   //
            //u8             m_u8;                    //
            //u16            m_u16;                   //
            //u32            m_u32;                   //
            //u64            m_u64;                   //
            //char*          m_char_ptr;              //
            size_t         m_size_t;
		    float          m_float;
		    double         m_double;
		    std::string*   m_std_string_ptr; // owned
		    void*          m_void_ptr;       // not owned
        } m_data;

        static_assert(sizeof(DataType) == sizeof(size_t)); // ensure DataType fits a size_t

    };
}