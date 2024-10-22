#pragma once
#include "Delegate.h"

namespace tools
{
    template<typename R, typename ...Args>
    struct Signal : Delegate<R, Args...>
    {
        Signal(Delegate<R, Args...> delegate = {} )
        : Delegate<R, Args...>( delegate )
        {}

        R emit(Args...args)
        { return call(args...); }
    };
}
//
// usage example
//
// // assign your own delegate once here, it will be called when this Slot changes
// SIGNAL(on_change, Event, Slot*);
//
#define SIGNAL( NAME, ... ) tools::Signal<void, ##__VA_ARGS__> NAME##_signal

#define CONNECT( SIGNAL, METHOD ) SIGNAL = decltype(SIGNAL)::from_method<&METHOD>(this)