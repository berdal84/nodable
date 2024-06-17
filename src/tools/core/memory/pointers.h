#pragma once
#include <type_traits>
#include <cstddef>

namespace tools
{
    // IMPLEMENTATION
    //---------------

    // Generic Wrapped Pointer
    template<class T, typename PtrT>
    class TWrappedPtr
    {
    public:
        using wrapped_t = PtrT;
        static constexpr bool uses_raw_ptr = std::is_same_v<T*, PtrT>;

        // checks
        static_assert(std::is_convertible_v<PtrT, T*>, "PtrT must be convertible to a raw pointer T*" );
        static_assert(std::is_convertible_v<T*, PtrT>, "raw pointer T* must be convertible to a PtrT" );

        TWrappedPtr(): TWrappedPtr(nullptr) {}
        ~TWrappedPtr() {}
        TWrappedPtr(PtrT ptr): m_ptr(ptr) {}
        TWrappedPtr(const TWrappedPtr& other): m_ptr(other.m_ptr) {}
        TWrappedPtr& operator=(const TWrappedPtr& other) { m_ptr(other.m_ptr); return *this; };
        TWrappedPtr& operator=(const PtrT ptr) { m_ptr = ptr; return *this; }
        bool operator!=(const TWrappedPtr& other) { return m_ptr != other.m_ptr; }
        bool operator!=(const PtrT ptr) { return m_ptr != ptr; }
        T* operator -> () { return m_ptr; }
        T& operator * () { return *m_ptr; }
        const T& operator * () const { return *m_ptr; }
    private:
        PtrT m_ptr;
    };

    template<class T>
    using WrappedRawPtr = TWrappedPtr<T, T*>;

    // Default WrappedPtr struct, user can override on a per-type basis.
    template<class T>
    struct WrappedPtr { using type = T*; };

    // Ptr is a shorthand for WrappedPtr<T>::type (which is T* or WrappedRawPtr<T>::type by default)
    // If you want your class to use a different pointer, declare your own template<class T> struct WrappedPtr
    // The only requirement is to declare a public "using type = ..." on it.
    template<class T>
    using Ptr = typename WrappedPtr<T>::type;

}