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
     * Union to store Variant data
     */
    union VariantData
    {
        VariantData():m_void_ptr(nullptr){}
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
        float          m_float;
        double         m_double;
        std::string*   m_std_string_ptr; // owned
        void*          m_void_ptr;       // not owned
    };
    static_assert(sizeof(VariantData) == sizeof(size_t)); // ensure VariantData fits a size_t


    /**
     * @brief This class can hold several types such as: bool, double, std::string, etc.. (see m_data member)
     */
	class Variant
    {
	public:
		Variant(std::shared_ptr<const R::MetaType> _type)
            : m_is_initialized(false)
            , m_is_defined(false)
            , m_meta_type(_type)
        {
        }

		~Variant();

		bool is_meta_type(std::shared_ptr<const R::MetaType> _meta_type)const;
        bool is_inititialized()const;
        bool is_defined() const { return m_is_defined; }
        void set_inititialized(bool _initialize);
		template<typename T>
        void set(T* _pointer)
        {
            if( !m_meta_type )
            {
                define_meta_type( R::get_meta_type<T*>() );
            }
            NODABLE_ASSERT( m_meta_type->has_qualifier(R::Qualifier::Pointer) )

            m_data.m_void_ptr = _pointer;
            m_is_defined = true;
        }
		void set(const Variant&);
		void set(const std::string&);
		void set(const char*);
		void set(double);
		void set(bool);

        template<typename T>
        void define_type()
        {
            NODABLE_ASSERT(m_meta_type == nullptr)
            define_meta_type(R::get_meta_type<T>());
        };
        void define_meta_type(std::shared_ptr<const R::MetaType> _type);

        std::shared_ptr<const R::MetaType> get_meta_type()const;

        template<typename T> T convert_to()const;

        // by reference
        template<typename T> operator const T*()const  { NODABLE_ASSERT(m_is_initialized) return reinterpret_cast<const T*>(m_data.m_void_ptr); }
        template<typename T> operator T*()             { NODABLE_ASSERT(m_is_initialized) return reinterpret_cast<T*>(m_data.m_void_ptr); }

		operator double*()        { NODABLE_ASSERT(m_is_initialized) return &m_data.m_double; }
        operator bool*()          { NODABLE_ASSERT(m_is_initialized) return &m_data.m_bool; }
        operator std::string* ()  { NODABLE_ASSERT(m_is_initialized) return m_data.m_std_string_ptr; }
        operator void* ()         { NODABLE_ASSERT(m_is_initialized) return m_data.m_void_ptr; }
        operator double&()        { NODABLE_ASSERT(m_is_initialized) return m_data.m_double; }
        operator bool&()          { NODABLE_ASSERT(m_is_initialized) return m_data.m_bool; }
        operator std::string& ()  { NODABLE_ASSERT(m_is_initialized) return *m_data.m_std_string_ptr; }

        // by value
        operator int()const;
        operator double()const;
        operator bool()const;
        operator std::string ()const;
        operator void* ()const;

    private:
        bool                               m_is_defined;
        bool                               m_is_initialized;
        std::shared_ptr<const R::MetaType> m_meta_type;
		VariantData                        m_data;
    };
}