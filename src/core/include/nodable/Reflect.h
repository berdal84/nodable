#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include "Log.h"

/**
 * Tiny replacement for Groufon's mirror library to work with c++11
 * Objective:
 * - ask isParentOf
 * - get/set members (TODO)
 * - automatic REFLECT_INHERITS declaration (TODO)
 */
namespace Nodable::Reflect
{
    /**
     * Initialize reflection, will perform runtime computation to precompute additional information.
     */
    static void Initialize()
    {
      LOG_VERBOSE("Reflect", "Initializing ...\n")
      // TODO
      LOG_VERBOSE("Reflect", "Initialized\n")
    }

    /**
     * Meta class to describe a class and get its information at runtime.
     */
    class Class
    {
    public:

        Class( const char* name)
        :
        m_name(name)
        {
            LOG_VERBOSE("Reflect", "new Class(%s)...\n", m_name)
        }

        ~Class(){}

        bool isChildOf( const Class* clss, bool selfCheck = true )
        {
            //LOG_VERBOSE("Reflect", "isChildOf calling (%s)...\n", m_name)

            auto found = std::find(m_parents.begin(), m_parents.end(), clss);

            if ( selfCheck && this == clss ) {
                return true;
            }

            // check direct
            if ( found != m_parents.end() )
                return true;

            // check indirect
            for ( Class* each : m_parents )
            {
                if ( each->isChildOf(clss, false) )
                {
                    return true;
                }
            }
            return false;
        };

        inline const char* getName() const
        {
            return m_name;
        }

        void addParent(Class* parent)
        {
            LOG_VERBOSE("Reflect", "%s addParent %s...\n", m_name, parent->m_name)
            m_parents.push_back(parent);
        }

        virtual void addChild(Class* child)
        {
          LOG_VERBOSE("Reflect", "%s addChild %s...\n", m_name, child->m_name)
            m_children.push_back(child);
        }

    private:
        const char* m_name;
        std::vector<Class*> m_parents;
        std::vector<Class*> m_children;
    };
}

/**
 * Must be inserted to start a reflection declaration, short version exist ex: REFLECT or REFLECT_WITH_INHERITANCE
 */
#define REFLECT_BEGIN( _Class, ... ) \
public:\
    \
    virtual Reflect::Class* getClass() const __VA_ARGS__ { \
      return _Class::GetClass();\
    } \
    \
    static Reflect::Class* GetClass() {  \
      static Reflect::Class* clss = ReflectClass(); \
      return clss; \
    } \
    \
    static Reflect::Class* ReflectClass() {   \
      LOG_MESSAGE( "Reflect", "ReflectClass %s\n", #_Class ) \
      Reflect::Class* _class = new Reflect::Class(#_Class);

/**
 * Must be inserted between REFLECT_BEGIN and REFLECT_END macro usage
 */
#define REFLECT_EXTENDS(_ParentClass) \
      /* _class is defined in REFLECT_BEGIN */ \
      LOG_MESSAGE( "Reflect", " - inherits %s \n", #_ParentClass ) \
      _class->addParent( _ParentClass::GetClass() ); \
      _ParentClass::GetClass()->addChild( _class );

/**
 * Must be added after any usage of REFLECT_BEGIN, can be placed after a REFLECT_INHERITS
 */
#define REFLECT_END \
      return _class; /* return for CreateClass() */ \
    }

/*
 * Short-end to reflect a class with minimal information (ex: name)
 */
#define REFLECT(_Class) \
    REFLECT_BEGIN( _Class ) \
    REFLECT_END

/**
 * Short-end to reflect a class with minimal information with inheritance information.
 */
#define REFLECT_DERIVED(_Class) \
    REFLECT_BEGIN( _Class, override )

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
#define REFLECT_DEFINE(_Class) static ::Nodable::Reflect::Class* _Class##_Reflect = ::Nodable::_Class::GetClass();