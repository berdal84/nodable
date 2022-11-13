#pragma once
#include <memory>

namespace ndbl {

    template<typename T>
    using s_ptr = std::shared_ptr<T>;

    template<typename T>
    using u_ptr = std::unique_ptr<T>;

    /*
     * Some functions present in C++14 and more recent are needed in Nodable.
     * But we don't need the whole std for 2 reasons:
     * - we only need a small part of it
     * - we want to be C++11 compatible
     */

    /** make_unique from https://herbsutter.com/gotw/_102/ */
    template<typename T, typename ...Args>
    u_ptr<T> make_unique( Args&& ...args )
    {
        return u_ptr<T>( new T( std::forward<Args>(args)... ) );
    }

    /** make_shared from https://herbsutter.com/gotw/_102/ */
    template<typename T, typename ...Args>
    s_ptr<T> make_shared( Args&& ...args )
    {
        return s_ptr<T>( new T( std::forward<Args>(args)... ) );
    }
}