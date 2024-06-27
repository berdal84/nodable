#pragma once

#include <string>
#include <array>
#include <memory> // std::shared_ptr

#include "tools/core/assertions.h"
#include "tools/core/types.h"
#include "tools/core/memory/memory.h"
#include "qword.h"
#include "type.h"

namespace tools
{
    /**
     * @brief This class can hold several types such as: bool, double, std::string, etc.. (see m_data property)
     */
	class variant
    {
    public:
        variant();
        ~variant();
        variant(const variant& other);

#define CONSTRUCTOR(type) variant(type val) { set(val); }
        CONSTRUCTOR(const std::string&)
        CONSTRUCTOR(const char*)
        CONSTRUCTOR(double)
        CONSTRUCTOR(i16_t)
        CONSTRUCTOR(i32_t)
        CONSTRUCTOR(bool)
        CONSTRUCTOR(null_t)
#undef CONSTRUCTOR

        qword*      data() const { return const_cast<qword*>(&m_data); }
        qword*      data() { return &m_data; }
        void        init_mem();
        void        release_mem(); // undo init_mem()
        bool        is_type(const tools::type*) const;
        bool        is_initialized() const { return m_flags & Flag_IS_MEM_INITIALIZED; }
        bool        is_defined() const { return m_flags & Flag_IS_DATA_DEFINED; }
        void        change_type(const type* _type);
        void        flag_defined();
        void        clear_data();
        void        set(void* ptr);
        void        set(const std::string& _value);
        void        set(const char* _value);
        void        set(null_t);
        void        set(double);
        void        set(bool);
        void        set(i16_t);
        void        set(i32_t);
        void        set(const variant&);
        const type* get_type()const { return m_type; }
        template<typename T>
        T           to()const;
        variant&    operator=(const variant& other);
        explicit operator double&();
        explicit operator u64_t&();
        explicit operator u32_t&();
        explicit operator i32_t&();
        explicit operator i16_t&();
        explicit operator bool&();
        explicit operator std::string& ();
        explicit operator double() const;
        explicit operator u64_t() const;
        explicit operator u32_t() const;
        explicit operator i32_t() const;
        explicit operator i16_t() const;
        explicit operator bool() const;
        explicit operator std::string() const;
        explicit operator const char*() const;
        explicit operator void* () const;

        template<typename T>
        T& as() { return (T)*this; }

        template<typename T>
        T as() const { return (T)*this; }
    private:
        enum Type // Internal Type enum to speedup switch/cases
        {
            Type_null = 0,
            Type_any, // "similar" to TypeScript's any.
            Type_bool,
            Type_double,
            Type_u64,
            Type_u32,
            Type_i32,
            Type_i16,
            Type_string,
            Type_pointer,

            Type_COUNT
        };

        static Type        type_to_enum(const tools::type* _type) ;

        typedef int Flags;
        enum Flag_
        {
            Flag_NONE               = 0,
            Flag_IS_DATA_DEFINED    = 1,      // True when user assigned a value to the variant's data
            Flag_IS_MEM_INITIALIZED = 1 << 1, // True when user initialized the variant with a given type (ex: with a std::string, it means the data.ptr points to a valid instance)
            Flag_ALLOWS_TYPE_CHANGE = 1 << 2  // True if variant's type can change over time, by default its strict (type can be set once).
        };

        Type          m_type_id  = Type_any;
        const type*   m_type     = nullptr;
        Flags         m_flags    = Flag_NONE;
        qword         m_data;
    };
}