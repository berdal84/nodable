#pragma once
#include "Delegate.h"
#include <algorithm>
#include <functional>
#include <vector>

namespace tools
{
    template<typename FunctionT = void()>
    struct Signal;
    
    template<typename FunctionT = void()>
    struct BroadcastSignal;

    using SimpleSignal = Signal<void()>;
    using SimpleBroadcastSignal = BroadcastSignal<void()>;

    //
    // Simple struct to call a unique delegate when triggered
    //
    // example:
    //      struct Number {
    //          Number(int n = 0): _m_value(n) {}
    //          void set_value(int n) const { _m_value = n; }
    //      private:
    //          int _m_value = 0;
    //      }
    //      Number number;
    //      tools::Signal<void(int)> signal_change;
    //      signal_change.connect<&Number::set_value>(&number); // &Number::set_value has same type as signal template argument
    //      signal_change.emit(42);
    //      assert(number.value() == 42);
    //
    //      // note that trying to connect another method will fail, see BroadcastSignal instead
    //      Number another_number;
    //      signal_change.connect<&Number::set_value>(&another_number);
    //
    template<typename R, typename ...Args>
    struct Signal<R(Args...)>
    {
        using DelegateT = Delegate<R(Args...)>; // the expected delegate type to connect with

        Signal() = default;
        Signal(const DelegateT& delegate)
        : _m_delegate(delegate)
        {}

        // TMethod: the address to a member function
        // object_ptr: the instance to call the member with
        template<auto TMethod>
        requires std::is_member_function_pointer_v<decltype(TMethod)>
        void connect(void* object_ptr)
        {
            auto delegate = DelegateT::template from_method<TMethod>(object_ptr);
            connect(delegate);
        }

        void connect(R(*function)(Args...) )
        {
            DelegateT delegate{function};
            connect(delegate);
        }
        
        void connect(const DelegateT& delegate)
        {
            ASSERT_DEBUG_ONLY(_m_delegate.is_null()); // Did you forgot to disconnect() before?
                                                        // Broadcasting not allowed. Did you called this multiple times on purpose?
                                                        // Use SignalN or SignalNR instead.
            ASSERT_DEBUG_ONLY(delegate.callable());     // Delegate should be callable..
            if( delegate.callable() )                   // ..and, in release we don't want to call it.
                _m_delegate = delegate;

            // we do not return an id since a Signal cannot have more than one connection (see BroadcastSignal).
        }

        void disconnect()
        {
            ASSERT_DEBUG_ONLY(!_m_delegate.is_null()); // Did you call connect() before?
            _m_delegate = {};
            ASSERT_DEBUG_ONLY(_m_delegate.is_null());
        }

        R emit(Args...args) const // emit signal to the listener, if any.
        {
            _m_delegate.call(args...); // calling a null delegate has no effect
        }

    private:
        DelegateT _m_delegate;
    };
    
    //
    // Similar to SignalR, but can be connected to multiple Delegates
    //
    template<typename R, typename ...Args>
    struct BroadcastSignal<R(Args...)>
    {
        using DelegateT = Delegate<R(Args...)>; // the expected delegate type to connect with

        BroadcastSignal() = default;
        BroadcastSignal(const BroadcastSignal&) = delete;

        // TMethod: the address to a member function
        // object_ptr: the instance to call the member with
        // Inserting multiple times the same method for the same pointer is undefined behavior.
        template<auto TMethod>
        requires std::is_member_function_pointer_v<decltype(TMethod)>
        void connect(void* object_ptr)
        {
            auto delegate = DelegateT::template from_method<TMethod>(object_ptr);
            VERIFY( delegate.callable(), "TMethod is not callable on object_ptr. Is object_ptr null?" );
            connect(delegate);
            // TODO: return an identifier/hash to disconnect with?
        }

        void connect(const DelegateT& delegate)
        {
            _m_delegate.emplace_back(std::move(delegate));
        }

        template<auto TMethod>
        requires std::is_member_function_pointer_v<decltype(TMethod)>
        bool disconnect(void* ptr) // Disconnects a given (TMethod, ptr) delegate
        {
            // We assume there is only one delegate per (TMethod, ptr) pair
            auto delegate = DelegateT::template from_method<TMethod>(ptr);
            auto it = std::find(_m_delegate.begin(), _m_delegate.end(), delegate);

            if ( it == _m_delegate.end() )
                return false;

            // Avoid useless memory copy by swapping with the last element
            auto last = _m_delegate.end() - 1;
            if ( it != last )
                std::iter_swap(it, last);

            _m_delegate.pop_back();

            return true;
        }

        R broadcast(Args...args) const // broadcast to all the listeners
        {
            for(const auto& delegate : _m_delegate )
                delegate.call(args...);
        }

    private:
        std::vector<DelegateT> _m_delegate;
    };
}
