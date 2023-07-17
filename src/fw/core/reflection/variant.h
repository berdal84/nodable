#pragma once

#include <string>
#include <array>
#include <memory> // std::shared_ptr

#include "../types.h"
#include "../assertions.h"
#include "qword.h"
#include "type.h"

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
            , m_type(type::any())
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

        variant(const variant&);
        variant(variant&&);
        ~variant();

        qword*  get_underlying_data() { return &m_data; }
        bool    is_initialized() const;
        bool    is_defined() const { return m_is_defined; }
        void    ensure_is_type(const type* _type);
        void    ensure_is_initialized(bool _initialize = true);
        void    flag_defined(bool _defined = true);
        void    reset_value();
        void    set(const std::string& _value);
        void    set(const char* _value);
        void    set(null_t) { ensure_is_type(type::null()); m_is_defined = false; };
        void    set(double);
        void    set(bool);
        void    set(i16_t);
        void    set(const variant&);

        template<typename T>
        void    set(T* _pointer)
        {
            ensure_is_type(type::get<decltype(_pointer)>());
            ensure_is_initialized();
            m_data.set<void*>(_pointer);
            m_is_defined = true;
        }

        const type* get_type()const;
        template<typename T> T convert_to()const;

        template<typename T>
        explicit operator const T*()const
        { FW_ASSERT(m_is_initialized) return reinterpret_cast<const T*>(m_data.ptr); }

        template<typename T>
        explicit operator T*()
        { FW_ASSERT(m_is_initialized) return reinterpret_cast<T*>(m_data.ptr); }

        variant operator=(const variant& other)
        {
            set(other);
            return *this;
        }

        // cast by pointer
		explicit operator double*()        { FW_ASSERT(m_is_initialized) return &m_data.d; }
        explicit operator i16_t *()        { FW_ASSERT(m_is_initialized) return &m_data.i16; }
        explicit operator bool*()          { FW_ASSERT(m_is_initialized) return &m_data.b; }
        explicit operator std::string* ()  { FW_ASSERT(m_is_initialized) return m_data.ptr_std_string; }
        explicit operator void* ()         { FW_ASSERT(m_is_initialized) return m_data.ptr; }

        // cast by address
        explicit operator double&()        { FW_ASSERT(m_is_initialized) return m_data.d; }
        explicit operator i16_t &()        { FW_ASSERT(m_is_initialized) return m_data.i16; }
        explicit operator bool&()          { FW_ASSERT(m_is_initialized) return m_data.b; }
        explicit operator std::string& ()  { FW_ASSERT(m_is_initialized) return *m_data.ptr_std_string; }

        // cast by copy
        explicit operator i16_t()const;
        explicit operator double()const;
        explicit operator bool()const;
        explicit operator std::string ()const;
        explicit operator void* ()const;
    private:
	    static const type*     normalize_type(const type *_type);

        bool            m_is_defined;
        bool            m_is_initialized;
        const type*     m_type;
        bool            m_type_change_allowed;
        qword           m_data;
    };
}