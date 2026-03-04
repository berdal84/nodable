#pragma once
#include "tools/core/assertions.h"

#define TOOLS_ENABLE_POINTER_COMPATIBILITY 1

namespace ndbl
{
    template<class T>
    struct ID                                                                    
    {                                                                               
        T* m_pointer = {};                                                         
        
        ID() = default;
        ID(T* pointer):m_pointer(pointer) {}

        const T* get() const
        {                                                      
            /* TODO: get pointer from a pool */                                     
            return m_pointer;                                                             
        }                                                                           
                                                                        
        T* get()
        {                                                                  
            return const_cast<T*>(const_cast<const ID<T>*>(this)->get());           
        }                                                                           
                                                                        
        const T* operator->() const
        {                                               
            ASSERT(m_pointer != nullptr);                                                 
            return get();                                                           
        }                                                                           
                                                                        
        T* operator->()
        {                                                           
            ASSERT(m_pointer != nullptr);                                                 
            return get();                                                           
        }                  

        template<class U>
        U get_as()
        {
           // TODO: add checks! 
           return static_cast<U>( get() );
        }

        template<class U>
        U get_as() const
        {
           // TODO: add checks! 
           return static_cast<U>( get() );
        }

        operator bool () const
        {
            return nullptr == this->m_pointer;
        }

#ifdef TOOLS_ENABLE_POINTER_COMPATIBILITY
        operator T* ()
        {
            return m_pointer;
        }

        operator const T* () const
        {
            return m_pointer;
        }
#endif

    };

    template<class A, class B>
    bool operator<(const ID<A> a, const ID<B> b)
    {
        return (u64_t)a.m_pointer < (u64_t)b.m_pointer;
    }

    template<class A, class B>
    bool operator==(const ID<A> a, const ID<B> b)
    {
        return a.get() == b.get();
    }

    template<class A>
    bool operator==(const ID<A> a, nullptr_t b)
    {
        return a.m_pointer == b;
    }

    template<class T>
    void mem_release(ID<T>& id)
    {
        T* temp      = id.get();
        id.m_pointer = nullptr;
        delete temp;
    }

    
#ifdef TOOLS_ENABLE_POINTER_COMPATIBILITY

    template<class A, class B>
    bool operator==(const ID<A> a, B* b)
    {
        return a.m_pointer == b;
    }

    template<typename T, typename U>
    ID<T> reinterpret_cast_id(ID<U> id)
    {
        return { reinterpret_cast<T*>(id.m_pointer) };
    }

    template<typename T, typename U>
    ID<T> static_cast_id(ID<U> id)
    {
        return { static_cast<T*>(id.m_pointer) };
    }

    template<typename T, typename U>
    ID<T> dynamic_cast_id(ID<U> id)
    {
        return { dynamic_cast<T*>(id.m_pointer) };
    }

#endif
}
