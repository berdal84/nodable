#pragma once

#include "bdc/String.hpp"

#include "bdc/Types.hpp"
#include "QWord.h"
#include "Type_Descriptor.h"

namespace tools
{
    /**
     * @brief This class can hold several types such as: bool, double, bdc::String, etc.. (see m_data get_value)
     */
	class Variant
    {
    public:
        Variant();
        ~Variant();

        Variant(const Variant& other);
        Variant(const bdc::String& val);
        Variant(const bdc::String val);
        Variant(double val);
        Variant(i16_t val) ;
        Variant(i32_t val);
        Variant(bool val);
        Variant(null val);

        void        set(void* /* pointer */);
        void        set(const bdc::String&);
        void        set(null);
        void        set(double);
        void        set(bool);
        void        set(i16_t);
        void        set(i32_t);
        void        set(const Variant&);

        const Type_Descriptor* get_type()const;
        bool        is_type(const Type_Descriptor*) const;
        void        change_type(const Type_Descriptor* _type);

        void        clear_data();
        const QWord*data() const; // get ptr to underlying data (QWord)

        template<typename T>
        T           to()const;
        Variant&    operator=(const Variant& other);
        explicit operator double&();
        explicit operator i32_t&();
        explicit operator i16_t&();
        explicit operator bool&();
        explicit operator bdc::String& ();
        explicit operator double() const;
        explicit operator i32_t() const;
        explicit operator i16_t() const;
        explicit operator bool() const;
        explicit operator bdc::String() const;
        explicit operator const bdc::String() const;
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
            Type_i16,
            Type_i32,
            Type_string,
            Type_ptr
        };

        void change_type(Type new_type);
        void init_mem();
        void release_mem(); // undo init_mem()
        bool is_mem_initialized() const;

        static Type                   type_to_enum(const Type_Descriptor*) ;
        static const tools::Type_Descriptor* enum_to_type(Type) ;

        typedef int Flags;
        enum Flag_
        {
            Flag_NONE                       = 0,
            Flag_OWNS_HEAP_ALLOCATED_MEMORY = 1 << 0, // True when dynamically allocated memory is owned by this Variant (ex: a bdc::String*)
            Flag_ALLOWS_TYPE_CHANGE         = 1 << 1  // True if Variant's type can change over time, by default its strict (type can be set once).
        };

        Type          m_type  = Type_any;
        Flags         m_flags = Flag_NONE;
        QWord         m_data  = {};
    };
}