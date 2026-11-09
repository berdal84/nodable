#pragma once

#ifdef BDC_ENABLE_LOGS
    #define BDC_LOG(...) printf(__VA_ARGS__)
    #define BDC_LOG_DEBUG(...) printf(__VA_ARGS__)
    #define BDG_LOG_FLUSH() std::cout << std::flush;
#else
    #define BDC_LOG(...)
    #define BDC_LOG_DEBUG(...)
    #define BDG_LOG_FLUSH()
#endif

#pragma once
#ifndef BDC_ASSERTIONS_ENABLE
#define BDC_ASSERTIONS_ENABLE true
#endif

#if BDC_ASSERTIONS_ENABLE

    #include <iostream> // for std::flush, std::cout

    #define BDC_ASSERT(expression, ... ) \
        if( !(expression) ) \
        { \
            printf("BDC_ASSERT( %s " __VA_OPT__(", \"" __VA_ARGS__ "\"") ") failed at " __FILE__ ":%i\n\n", #expression, __LINE__ ); \
            std::cout << std::flush; \
            std::abort(); \
        }


#else // BDC_ASSERTIONS_ENABLE

    #define BDC_ASSERT(...)

#endif // BDC_ASSERTIONS_ENABLE


namespace bdc
{
    template <typename Function_Type>
    struct At_Destruction_Caller
    {
        Function_Type func;

        At_Destruction_Caller(Function_Type&& f)
        : func(std::move(f))
        {}

        ~At_Destruction_Caller()
        {
            func();
        }
    };
}

// usage:
//  {
//      char* data = malloc(...);
//      DEFER( free(data); )
//
//      // do stuff...
//
//  } <- data gets free there
//
//  or ..
//
//  {
//      char* data1 = malloc(...);
//      char* data2 = malloc(...);
//
//      DEFER({
//          free(data1);
//          free(data2);
//      })
//
//      // do stuff...
//
//  }  <- data1 then data2 gets free there

#define DEFER( ... ) \
    auto _defer_##__LINE__ = bdc::At_Destruction_Caller([&] { __VA_ARGS__; });

