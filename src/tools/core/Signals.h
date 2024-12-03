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
#ifdef TOOLS_DEBUG
            VERIFY( !disconnect( delegate._object_ptr ), "A single object can't be connected twice to the same signal yet" )
#endif
            _delegate.emplace_back(delegate);
        }

        template<auto TMethod>
        inline void connect(void* object_ptr) // Shorthand
        { connect( TDelegate::template from_method<TMethod>(object_ptr) ); }

        inline bool disconnect(void* ptr) // Disconnects any delegate related to this pointer. TODO: we should have an indexed storage using a hash per delegate based on TMethod and _object_ptr instead
        {
            auto it = _delegate.rbegin();
            while ( it != _delegate.rend() )
            {
                if ( it->_object_ptr == ptr )
                {
                    std::advance(it, 1); // can't erase a reverse iterator
                    _delegate.erase( it.base() );
                    return true;
                }
                ++it;
            }
            return false;
        }

        inline R emit(Args...args)
        {
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

#define CONNECT( SIGNAL, METHOD_PTR, OBJ_PTR ) SIGNAL.connect<METHOD_PTR>( OBJ_PTR )
#define DISCONNECT( SIGNAL, OBJ_PTR ) SIGNAL.disconnect( OBJ_PTR )