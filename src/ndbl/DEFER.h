#pragma once

template <typename Function_Type>
struct At_Destruction_Caller
{
    Function_Type func;

    At_Destruction_Caller(Function_Type f)
    : func(std::move(f))
    {}

    ~At_Destruction_Caller()
    {
        func();
    }
};

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

#define DEFER( code ) \
    auto _defer_##__LINE__ = At_Destruction_Caller([&] { code });