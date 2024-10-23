#pragma once
#include "Delegate.h"
#include <vector>

namespace tools
{
    //
    // Simple struct to call a list of delegates that have been connected
    //
    template<typename R, typename ...Args>
    struct Signal
    {
        typedef Delegate<R, Args...> TDelegate;

        Signal(){}

        inline void connect(TDelegate delegate)
        {
            LOG_VERBOSE("Signal", "connect() was called");
            _delegate.emplace_back(delegate);
        }

        template<auto TMethod>
        inline void connect(void* object_ptr) // Shorthand
        { connect( TDelegate::template from_method<TMethod>(object_ptr) ); }

        inline R emit(Args...args)
        {
            LOG_VERBOSE("Signal", "emit() was called");
            for( auto& delegate : _delegate )
                delegate.call(args...);
        }

    private:
        std::vector<TDelegate> _delegate;
    };
}
//
// usage example
//
// // assign your own delegate once here, it will be called when this Slot changes
// SIGNAL(on_change, Event, Slot*);
//
#define SIGNAL( NAME, ... ) tools::Signal<void, ##__VA_ARGS__> NAME

#define CONNECT( SIGNAL, METHOD_PTR ) SIGNAL.connect<METHOD_PTR>( this )