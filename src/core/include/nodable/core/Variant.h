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
	class Variant {
    public:
        using QWord    = assembly::QWord;

        Variant(R::Meta_t_csptr _type)
                : m_is_initialized(false)
                , m_is_defined(false)
                , m_meta_type(_type) {
        }
        ~Variant();

        QWord*  get_data_ptr();
        bool    is_meta_type(R::Meta_t_csptr _meta_type) const;
        bool    is_initialized() const;
        bool    is_defined() const { return m_is_defined; }
        void    set_initialized(bool _initialize);
        template<typename T>
        void set(T *_pointer)
        {
            if (!m_meta_type) define_meta_type(R::meta<T *>());
            NODABLE_ASSERT(m_meta_type->has_qualifier(R::Qualifier::Pointer))
            m_data.ptr = _pointer;
            m_is_defined = true;
        }
        template<typename T> //-------- for any fundamental types
        void set(T _value)
        {
            ensure_is_initialized_as<T>();
            R::set_union(m_data, _value);
            m_is_defined = true;
        }
        void set(const Variant &);
        void set(const std::string &);
        void set(const char *);
        void force_defined_flag(bool _value);
        template<typename T>
        void define_type()
        {
            NODABLE_ASSERT(m_meta_type == nullptr)
            define_meta_type(R::meta<T>());
        };
        void define_meta_type(R::Meta_t_csptr _type);
        R::Meta_t_csptr get_meta_type()const;
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
        R::Meta_t_csptr m_meta_type;
        QWord           m_data;

        template<typename T>
        void ensure_is_initialized_as()
        {
            using clean_T = typename std::remove_reference<T>::type; // skip reference
            auto meta_t   = R::meta<clean_T>();
            if( !m_is_initialized) define_meta_type( meta_t );
            NODABLE_ASSERT( is_meta_type( meta_t ) )
        };
    };
}