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
     * Meta class to describe a class and get its information at runtime.
     */
    class Class
    {
    public:

        Class( const char* name)
        :
        m_name(name)
        {
            LOG_VERBOSE("Reflect", "Constructor calling (%s)...\n", m_name)
        }

        ~Class(){}

        bool isChildOf( const Class* clss, bool selfCheck = true )
        {
            LOG_VERBOSE("Reflect", "isChildOf calling (%s)...\n", m_name)

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
            LOG_VERBOSE("Reflect", "addParent calling (%s)...\n", m_name)
            m_parents.push_back(parent);
            LOG_VERBOSE("Reflect", "addParent called (%s)...\n", m_name)
        }

        virtual void addChild(Class* child)
        {
            LOG_VERBOSE("Reflect", "addChild calling (%s)...\n", m_name)
            m_children.push_back(child);
            LOG_VERBOSE("Reflect", "addChild called (%s)...\n", m_name)
        }

        /**
         * Initialize reflection, will perform runtime computation to precompute additional information.
         */
        //static void Initialize()
        //{
            // TODO: here I would like to try to create parent-child links automaticaly using std::is_base_of<A, B>::value
        //}

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
    virtual std::string getClassName() const \
    {\
         return _Class::GetClass()->getName();\
    }\
    \
    virtual Reflect::Class* getClass() const { \
      return _Class::GetClass();\
    } \
    \
    static Reflect::Class* GetClass() {  \
      static Reflect::Class* clss = CreateClass(); \
      return clss; \
    } \
    \
    static Reflect::Class* CreateClass() {  \
      Reflect::Class* _class = new Reflect::Class(#_Class);

/**
 * Must be inserted between REFLECT_BEGIN and REFLECT_END macro usage
 */
#define REFLECT_INHERITS( _ParentClass ) \
      /* _class is defined in REFLECT_BEGIN */ \
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
#define REFLECT( _Class ) \
    REFLECT_BEGIN( _Class ) \
    REFLECT_END

/**
 * Short-end to reflect a class with minimal information with inheritance information.
 */
#define REFLECT_WITH_INHERITANCE( _Class, _ParentClass ) \
    REFLECT_BEGIN( _Class )    \
    REFLECT_INHERITS( _ParentClass ) \
    REFLECT_END