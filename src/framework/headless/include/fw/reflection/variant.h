#pragma once

#include <string>
#include <array>
#include <memory> // std::shared_ptr

#include <fw/types.h> // for constants and forward declarations
#include <fw/assertions.h>
#include <fw/reflection/qword.h>
#include <fw/reflection/type.h>

namespace fw
{
    /**
     * @brief This class can hold several types such as: bool, double, std::string, etc.. (see m_data property)
     */
	class variant {
    public:
        variant()
            : m_is_initialized(false)
            , m_is_defined(false)
            , m_type(type::any)
            , m_type_change_allowed(false) // for now, variant can change type once
        {}

#define CONSTRUCTOR(type) variant(type val): variant() { set(val); }
        CONSTRUCTOR(void *)
        CONSTRUCTOR(const std::string&)
        CONSTRUCTOR(const char*)
        CONSTRUCTOR(double)
        CONSTRUCTOR(i16_t)
        CONSTRUCTOR(bool)
        CONSTRUCTOR(null_t)
#undef CONSTRUCTOR

        variant& operator=(const variant& );
        ~variant();

        qword&  get_underlying_data() { return m_data; }
        bool    is_initialized() const;
        bool    is_defined() const { return m_is_defined; }
        void    ensure_is_type(type _type);
        void    ensure_is_initialized(bool _initialize = true);
        void    flag_defined(bool _defined = true);
        void    reset_value();
        void    set(const std::string& _value);
        void    set(const char* _value);

        template<typename T>
        void    set(T* _pointer)
        {
            ensure_is_type(type::get<decltype(_pointer)>());
            ensure_is_initialized();
            m_data.set<void*>(_pointer);
            m_is_defined = true;
        }

        void    set(null_t) { ensure_is_type(type::null); m_is_defined = false; };
        void    set(double);
        void    set(bool);
        void    set(i16_t);
        const type & get_type()const;
        template<typename T> T convert_to()const;

        template<typename T> operator const T*()const  { NDBL_ASSERT(m_is_initialized) return reinterpret_cast<const T*>(m_data.ptr); }
        template<typename T> operator T*()             { NDBL_ASSERT(m_is_initialized) return reinterpret_cast<T*>(m_data.ptr); }

        // cast by pointer
		operator double*()        { NDBL_ASSERT(m_is_initialized) return &m_data.d; }
		operator i16_t *()        { NDBL_ASSERT(m_is_initialized) return &m_data.i16; }
        operator bool*()          { NDBL_ASSERT(m_is_initialized) return &m_data.b; }
        operator std::string* ()  { NDBL_ASSERT(m_is_initialized) return static_cast<std::string*>(m_data.ptr); }
        operator void* ()         { NDBL_ASSERT(m_is_initialized) return m_data.ptr; }

        // cast by address
        operator double&()        { NDBL_ASSERT(m_is_initialized) return m_data.d; }
        operator i16_t &()        { NDBL_ASSERT(m_is_initialized) return m_data.i16; }
        operator bool&()          { NDBL_ASSERT(m_is_initialized) return m_data.b; }
        operator std::string& ()  { NDBL_ASSERT(m_is_initialized) return *static_cast<std::string*>(m_data.ptr); }

        // cast by copy
        operator i16_t()const;
        operator double()const;
        operator bool()const;
        operator std::string ()const;
        operator void* ()const;

    private:
	    static type     clean_type(const type&);

        bool            m_is_defined;
        bool            m_is_initialized;
        type            m_type;
        bool            m_type_change_allowed;
        qword           m_data;
    };
}