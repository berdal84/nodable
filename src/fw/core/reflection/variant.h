#pragma once

#include <string>
#include <array>
#include <memory> // std::shared_ptr

#include "../types.h"
#include "../assertions.h"
#include "qword.h"
#include "type.h"
#include "core/Pool.h"

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
        CONSTRUCTOR(const std::string&)
        CONSTRUCTOR(const char*)
        CONSTRUCTOR(double)
        CONSTRUCTOR(i16_t)
        CONSTRUCTOR(i32_t)
        CONSTRUCTOR(bool)
        CONSTRUCTOR(null_t)
#undef CONSTRUCTOR

        variant(const variant&);
        variant(variant&&);
        ~variant();

        qword*      data() { return &m_data; }
        bool        is_initialized() const;
        bool        is_defined() const { return m_is_defined; }
        void        ensure_is_type(const type* _type);
        void        ensure_is_initialized(bool _initialize = true);
        void        flag_defined(bool _defined = true);
        void        reset_value();
        template<typename T>
        void        set(ID<T> id);
        void        set(const std::string& _value);
        void        set(const char* _value);
        void        set(null_t);;
        void        set(double);
        void        set(bool);
        void        set(i16_t);
        void        set(i32_t);
        void        set(const variant&);
        const type* get_type()const;
        template<typename T>
        T           to()const;
        variant     operator=(const variant& other);
        explicit operator double&();
        explicit operator u32_t&();
        explicit operator i32_t&();
        explicit operator i16_t&();
        explicit operator bool&();
        explicit operator std::string& ();
        explicit operator double() const;
        explicit operator u32_t() const;
        explicit operator i32_t() const;
        explicit operator i16_t() const;
        explicit operator bool() const;
        explicit operator std::string() const;
        explicit operator const char*() const;

        template<typename T>
        explicit operator ID<T> () const
        {
            using underlying_type = typename ID<T>::id_t;
            return (underlying_type)*this;
        }

        template<typename T>
        T& as() { return (T)*this; }

        template<typename T>
        T as() const { return (T)*this; }
    private:
	    static const type* normalize_type(const type *_type);

        bool        m_is_defined;
        bool        m_is_initialized;
        const type* m_type;
        bool        m_type_change_allowed;
        qword       m_data;
    };


    template<typename T>
    void variant::set(ID<T> _id)
    {
        ensure_is_type(type::get<ID<T>>());
        ensure_is_initialized();
        m_data.u32 = _id.m_value;
        flag_defined();
    }
}