#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include <nodable/Log.h>

/**
 * Tiny replacement for Groufon's mirror library to work with c++11
 * Objective:
 * - ask isParentOf
 * - reflect base types (string, double, bool, etc.)
 * - get/set members (TODO)
 * - automatic REFLECT_INHERITS declaration (TODO)
 */
namespace Nodable::Reflect
{
    /**
     * Meta structures to describe a type and get its information at runtime.
     */
    enum Type
    {
        Type_Unknown      = 0,
        Type_Pointer      = 0b00000001, // multi-pointers not allowed.
        Type_Reference    = 0b00000010, // multi-refs not allowed.
        // = 0b00000100, for future use
        Type_Boolean      = 0b00001000,
        Type_Double       = 0b00010000,
        Type_String       = 0b00100000,
        Type_Object       = 0b01000000,
        Type_Any          = Type_Boolean | Type_Double | Type_String,

        Type_COUNT        = 0b10000000,

        Type_Boolean_Ptr  = Type_Boolean | Type_Pointer,
        Type_Double_Ptr   = Type_Double | Type_Pointer,
        Type_String_Ptr   = Type_String | Type_Pointer,
        Type_Object_Ptr   = Type_Object | Type_Pointer,
        Type_Any_Ptr      = Type_Any | Type_Pointer,

        Type_Boolean_Ref  = Type_Boolean | Type_Reference,
        Type_Double_Ref   = Type_Double | Type_Reference,
        Type_Object_Ref   = Type_Object | Type_Reference,
        Type_String_Ref   = Type_String | Type_Reference,
        Type_Any_Ref      = Type_Any | Type_Reference,
    };


    inline static std::string to_string( Type _type )
    {
        switch( _type )
        {
            case Type_Object:	{return "object";}
            case Type_String:	{return "string";}
            case Type_Double:	{return "double";}
            case Type_Boolean: 	{return "boolean";}
            default:			{return "unknown";}
        }
    }

    inline static bool is_convertible( Type left, Type right )
    {
        return (( left & Type_Any ) & ( right & Type_Any )) != 0; // check if type are matching (ignoring ref/pointer)
    }

    inline static bool is_pointer( Type left)
    {
        return (left & Type_Pointer ) != 0;
    }

    inline static bool is_reference( Type left)
    {
        return (left & Type_Reference ) != 0;
    }

    /*
        The role of these templates is to help conversion between cpp type to nodable Type

        double  <--> Type_Double
        bool    <--> Type_Boolean
        double* <--> Type_Double_Ptr
    */
    template<typename T>
    struct to_Type;

    template<Type T>
    struct from_Type;

    #define REFLECT_DEFINE_TYPE( cpp_type, reflect_type ) \
        template<> \
        struct Reflect::to_Type<cpp_type> { \
            static constexpr Reflect::Type type = reflect_type; \
            static constexpr const char* type_name = #reflect_type; \
            static constexpr const char* cpp_type_name = #cpp_type; \
        }; \
        template<> \
        struct Reflect::from_Type<Reflect::reflect_type> { \
            using type = cpp_type; \
        };\
        template<> \
        struct Reflect::from_Type<Reflect::reflect_type##_Ptr> { \
            using type = cpp_type*; \
        };\
        template<> \
        struct Reflect::from_Type<Reflect::reflect_type##_Ref> { \
            using type = cpp_type&; \
        };\
        template<> \
        struct Reflect::to_Type<cpp_type&> { \
            static constexpr Type type = (Type)(reflect_type | Type_Reference); \
            static constexpr const char* type_name = #reflect_type"&"; \
            static constexpr const char* cpp_type_name = #cpp_type"&"; \
        }; \
        template<> \
        struct Reflect::to_Type<cpp_type*> { \
            static constexpr Type type = (Type)(reflect_type | Type_Pointer); \
            static constexpr const char* type_name = #reflect_type"*"; \
            static constexpr const char* cpp_type_name = #cpp_type"*"; \
        };

    REFLECT_DEFINE_TYPE(double, Type_Double )
    REFLECT_DEFINE_TYPE(std::string, Type_String )
    REFLECT_DEFINE_TYPE(bool, Type_Boolean )

    /**
     * Initialize reflection, will perform runtime computation to precompute additional information.
     */
    static void Initialize()
    {
        LOG_VERBOSE("Reflect", "Initializing ...\n")

        LOG_VERBOSE("Reflect", "Initialized\n")
    }

    /**
     * Meta class to describe a class and get its information at runtime.
     */
    class Class
    {
    public:

        Class( const char* _name)
        :
        m_name(_name)
        {
            LOG_VERBOSE("Reflect", "new Class(%s)...\n", _name)
        }

        ~Class(){}

        bool is_child_of(const Class* _possible_parent_class, bool _selfCheck = true)const
        {
            //LOG_VERBOSE("Reflect", "isChildOf calling (%s)...\n", m_name)

            auto found = std::find(m_parents.begin(), m_parents.end(), _possible_parent_class);

            if ( _selfCheck && this == _possible_parent_class ) {
                return true;
            }

            // check direct
            if ( found != m_parents.end() )
                return true;

            // check indirect
            for ( Class* each : m_parents )
            {
                if (each->is_child_of(_possible_parent_class, true) )
                {
                    return true;
                }
            }
            return false;
        };

        inline const char* get_name() const
        {
            return m_name;
        }

        inline void add_parent(Class *_parent) // can add multiple parent in case of multiple inheritance
        {
            LOG_VERBOSE("Reflect", "%s addParent %s...\n", m_name, _parent->m_name)
            m_parents.push_back(_parent);
        }

        inline void add_child(Class* _child)
        {
          LOG_VERBOSE("Reflect", "%s addChild %s...\n", m_name, _child->m_name)
            m_children.push_back(_child);
        }

        template<class T> inline bool is() const { return is_child_of(T::Get_class(), true);}
        template<class T> inline bool is_not() const { return !is_child_of(T::Get_class(), true); }

    private:
        const char* m_name;
        std::vector<Class*> m_parents;
        std::vector<Class*> m_children;
    };


    /**
     * Cast a _source instance from Src to Dst type.
     * If not possible returns nullptr
     *
     * @tparam Src the source type
     * @tparam Dst
     * @param _source
     * @return
     */
    template<class Dst, class Src>
    inline Dst* cast_pointer(Src *_source)
    {
        if(_source->get_class()->is_child_of(Dst::Get_class()))
            return dynamic_cast<Dst*>(_source);
        return nullptr;
    };
}

/**
 * Must be inserted to start a reflection declaration, short version exist ex: REFLECT or REFLECT_WITH_INHERITANCE
 */
#define REFLECT_BEGIN( _CLASS, ... ) \
public:\
    \
    virtual Reflect::Class* get_class() const __VA_ARGS__ { \
      return _CLASS::Get_class();\
    } \
    \
    static Reflect::Class* Get_class() {  \
      static Reflect::Class* clss = _CLASS::Reflect_class(); \
      return clss; \
    } \
    \
    static Reflect::Class* Reflect_class() {   \
      LOG_MESSAGE( "Reflect", "ReflectClass %s\n", #_CLASS ) \
      Reflect::Class* clss = new Reflect::Class(#_CLASS);

/**
 * Must be inserted between REFLECT_BEGIN and REFLECT_END macro usage
 */
#define REFLECT_EXTENDS(_PARENT_CLASS) \
      /* _class is defined in REFLECT_BEGIN */ \
      LOG_MESSAGE( "Reflect", " - inherits %s \n", #_PARENT_CLASS ) \
      clss->add_parent( _PARENT_CLASS::Get_class() ); \
      _PARENT_CLASS::Get_class()->add_child( clss );

/**
 * Must be added after any usage of REFLECT_BEGIN, can be placed after a REFLECT_INHERITS
 */
#define REFLECT_END \
      return clss; /* return for CreateClass() */ \
    }

/*
 * Short-end to reflect a class with minimal information (ex: name)
 */
#define REFLECT( _CLASS ) \
    REFLECT_BEGIN( _CLASS ) \
    REFLECT_END

/**
 * Short-end to reflect a class with minimal information with inheritance information.
 */
#define REFLECT_DERIVED(_CLASS ) \
    REFLECT_BEGIN( _CLASS, override )

/**
 * Must be added to your class *.cpp file in order to generate MetaClass before main() starts.
 * note this is not always required, for example with
 *
 * class A { ... };
 * class B : class A { ... }
 *
 * here A DEFINITION can be omitted since B DEFINITION will also define A.
 *
 * It works with N level(s) of inheritance too.
 * A <- B <- C <- D, here only D needs to be explicitly defined in it's cpp.
 *
 */
#define REFLECT_DEFINE_CLASS( _CLASS ) \
    static ::Nodable::Reflect::Class* _CLASS##_Reflect = ::Nodable::_CLASS::Get_class(); \
    REFLECT_DEFINE_TYPE( _CLASS, Type_Object ) // for conversions cpp_type <--> reflect_type