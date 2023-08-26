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
    using pool::ID;

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
        operator    double&();
        operator    i32_t&();
        operator    i16_t&();
        operator    bool&();
        operator    std::string& ();
        operator    double() const;
        operator    i32_t() const;
        operator    i16_t() const;
        operator    bool() const;
        operator    std::string() const;
        operator    const char*() const;

        template<typename T>
        T& as() { return *this; }

        template<typename T>
        T as() const { return *this; }
    private:
	    static const type* normalize_type(const type *_type);

        bool        m_is_defined;
        bool        m_is_initialized;
        const type* m_type;
        bool        m_type_change_allowed;
        qword       m_data;
    };


    template<typename T>
    void variant::set(ID<T> id)
    {
        ensure_is_type(type::get<ID<T>>());
        ensure_is_initialized();
        m_data.i32 = (i32_t)id;
        flag_defined();
    }
}