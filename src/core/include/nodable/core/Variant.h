#pragma once

#include <string>
#include <array>
#include <memory> // std::shared_ptr

#include <nodable/core/types.h> // for constants and forward declarations
#include <nodable/core/reflection/reflection>
#include <nodable/core/assertions.h>
#include <nodable/core/assembly/QWord.h>

namespace Nodable
{
    // forward declarations
    class Node;

    /**
     * @brief This class can hold several types such as: bool, double, std::string, etc.. (see m_data member)
     */
	class Variant {
    public:
        using QWord    = assembly::QWord;

        Variant(type _initial_type = type::any )
                : m_is_initialized(false)
                , m_is_defined(false)
                , m_type(_initial_type) {
        }
        ~Variant();

        QWord*  get_data_ptr();
        bool    is_initialized() const;
        bool    is_defined() const { return m_is_defined; }
        void    set_initialized(bool _initialize);
        template<typename T>
        void set(T *_pointer)
        {
            ensure_is_initialized_as<T*>();
            NODABLE_ASSERT(m_type.is_ptr())
            m_data.ptr = _pointer;
            m_is_defined = true;
        }
        template<typename T> //-------- for any fundamental types
        void set(T _value)
        {
            ensure_is_initialized_as<T>();
            set_union(m_data, _value);
            m_is_defined = true;
        }
        void set(const Variant &);
        void set(const std::string &);
        void set(const char *);
        void force_defined_flag(bool _value);
        template<typename T>
        void define_type()
        {
            NODABLE_ASSERT(m_type == type::null)
            define_type(type::get<T>());
        };
        void define_type(type _type);
        const type & get_type()const;
        template<typename T> T convert_to()const;

        template<typename T> operator const T*()const  { NODABLE_ASSERT(m_is_defined) return reinterpret_cast<const T*>(m_data.ptr); }
        template<typename T> operator T*()             { NODABLE_ASSERT(m_is_defined) return reinterpret_cast<T*>(m_data.ptr); }

        // cast by pointer
		operator double*()        { NODABLE_ASSERT(m_is_initialized) return &m_data.d; }
		operator i16_t *()        { NODABLE_ASSERT(m_is_initialized) return &m_data.i16; }
        operator bool*()          { NODABLE_ASSERT(m_is_initialized) return &m_data.b; }
        operator std::string* ()  { NODABLE_ASSERT(m_is_initialized) return static_cast<std::string*>(m_data.ptr); }
        operator void* ()         { NODABLE_ASSERT(m_is_initialized) return m_data.ptr; }

        // cast by address
        operator double&()        { NODABLE_ASSERT(m_is_initialized) return m_data.d; }
        operator i16_t &()        { NODABLE_ASSERT(m_is_initialized) return m_data.i16; }
        operator bool&()          { NODABLE_ASSERT(m_is_initialized) return m_data.b; }
        operator std::string& ()  { NODABLE_ASSERT(m_is_initialized) return *static_cast<std::string*>(m_data.ptr); }

        // cast by copy
        operator i16_t()const;
        operator double()const;
        operator bool()const;
        operator std::string ()const;
        operator void* ()const;

    private:
        bool            m_is_defined;
        bool            m_is_initialized;
        type         m_type;
        QWord           m_data;

        template<typename T>
        void ensure_is_initialized_as()
        {
            using clean_T = typename std::remove_reference<T>::type; // skip reference
            auto type   = type::get<clean_T>();
            if( !m_is_initialized) define_type(type);
            NODABLE_ASSERT( m_type == type )
        };
    };
}