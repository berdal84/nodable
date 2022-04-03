#pragma once

#include <string>
#include <array>
#include <memory> // std::shared_ptr

#include <nodable/core/types.h> // for constants and forward declarations
#include <nodable/core/reflection/R.h>
#include <nodable/core/assertions.h>
#include <nodable/core/assembly/QWord.h>

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
		Variant(std::shared_ptr<const R::MetaType> _type)
            : m_is_initialized(false)
            , m_is_defined(false)
            , m_meta_type(_type)
        {
        }

		~Variant();

		bool is_meta_type(std::shared_ptr<const R::MetaType> _meta_type)const;
        bool is_initialized()const;
        bool is_defined() const { return m_is_defined; }
        void set_initialized(bool _initialize);
		template<typename T>
        void set(T* _pointer)
        {
            if( !m_meta_type )
            {
                define_meta_type( R::get_meta_type<T*>() );
            }
            NODABLE_ASSERT( m_meta_type->has_qualifier(R::Qualifier::Pointer) )

            m_data.ptr   = _pointer;
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
        template<typename T> operator const T*()const  { NODABLE_ASSERT(m_is_defined) return reinterpret_cast<const T*>(m_data.ptr); }
        template<typename T> operator T*()             { NODABLE_ASSERT(m_is_defined) return reinterpret_cast<T*>(m_data.ptr); }

		operator double*()        { NODABLE_ASSERT(m_is_initialized) return &m_data.d; }
		operator i16_t *()        { NODABLE_ASSERT(m_is_initialized) return &m_data.i16; }
        operator bool*()          { NODABLE_ASSERT(m_is_initialized) return &m_data.b; }
        operator std::string* ()  { NODABLE_ASSERT(m_is_initialized) return (std::string*)m_data.ptr; }
        operator void* ()         { NODABLE_ASSERT(m_is_initialized) return m_data.ptr; }
        operator double&()        { NODABLE_ASSERT(m_is_initialized) return m_data.d; }
        operator i16_t &()        { NODABLE_ASSERT(m_is_initialized) return m_data.i16; }
        operator bool&()          { NODABLE_ASSERT(m_is_initialized) return m_data.b; }
        operator std::string& ()  { NODABLE_ASSERT(m_is_initialized) return *(std::string*)m_data.ptr; }

        // by value
        operator int()const;
        operator double()const;
        operator bool()const;
        operator std::string ()const;
        operator void* ()const;

        void force_defined_flag(bool _value);

        assembly::QWord* get_data_ptr();

    private:
        bool                               m_is_defined;
        bool                               m_is_initialized;
        std::shared_ptr<const R::MetaType> m_meta_type;
        assembly::QWord                  m_data;
    };
}