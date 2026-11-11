#pragma once

#ifndef BDC_LOG_OUTPUT_PATH
#define BDC_LOG_OUTPUT_PATH "bdc.logs.txt"
#endif

#ifdef BDC_ENABLE_LOGS
    #include <print>
    #include <fstream>

    namespace bdc 
    {
        static std::ofstream log_output_file{ BDC_LOG_OUTPUT_PATH };
    }

    #define BDC_LOG(...)       do { std::print(bdc::log_output_file, __VA_ARGS__); std::print(std::cout, __VA_ARGS__); } while (0)
    #define BDC_LOG_DEBUG(...) do { std::print(bdc::log_output_file, __VA_ARGS__); } while (0)
    #define BDC_LOG_FLUSH()    do { bdc::log_output_file << std::flush; std::cout << std::flush; } while (0)
#else
    #define BDC_LOG(...)
    #define BDC_LOG_DEBUG(...)
    #define BDC_LOG_FLUSH()
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
            BDC_LOG("BDC_ASSERT( {} " __VA_OPT__(", \"" __VA_ARGS__ "\"") ") failed at " __FILE__ ":{}\n\n", #expression, __LINE__ ); \
            BDC_LOG_FLUSH(); \
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

