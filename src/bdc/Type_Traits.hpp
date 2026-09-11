#pragma once

#include <type_traits>  // for std::is_default_constructible_v and co.
#include <utility>      // for std::forward

namespace bdc
{
    //
    // Return a type referencing Type as a T& or T*
    // ex:
    //  std::string => std::string&
    //  const char* => const char* 
    //  u64_t       => u64_t&
    //
    template<typename Type>
    using Ref_Or_Ptr = std::conditional_t<
        std::is_pointer_v<Type>,
            std::remove_pointer_t<Type>*,
            std::remove_reference_t<Type>&
    >;
}