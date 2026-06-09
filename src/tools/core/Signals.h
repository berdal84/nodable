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
    struct Broadcast_Signal;

    using Simple_Signal = Signal<void()>;
    using Simple_Broadcast_Signal = Broadcast_Signal<void()>;

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
    //      // note that trying to connect another method will fail, see Broadcast_Signal instead
    //      Number another_number;
    //      signal_change.connect<&Number::set_value>(&another_number);
    //
    template<typename Result_Type, typename ...Arg_Types>
    struct Signal<Result_Type(Arg_Types...)>
    {
        using Delegate_Type = Delegate<Result_Type(Arg_Types...)>; // the expected delegate type to connect with

        Signal() = default;
        Signal(const Delegate_Type& delegate)
        : _m_delegate(delegate)
        {}

        // TMethod: the address to a member function
        // object_ptr: the instance to call the member with
        template<auto Method_Type>
        void connect(void* object_ptr)
        {
            auto delegate = Delegate_Type::template from_method<Method_Type>(object_ptr);
            connect(delegate);
        }
      
        template<typename Struct_Type, auto CStyle_Method_Type>
        void connect(Struct_Type* object_ptr)
        {
            auto delegate = Delegate_Type::template from_cstyle_method<Struct_Type, CStyle_Method_Type>(object_ptr);
            connect(delegate);
        }

        void connect(Result_Type(*function)(Arg_Types...) )
        {
            Delegate_Type delegate{function};
            connect(delegate);
        }
        
        void connect(const Delegate_Type& delegate)
        {
            ASSERT_DEBUG_ONLY(_m_delegate.is_null()); // Did you forgot to disconnect() before?
                                                        // Broadcasting not allowed. Did you called this multiple times on purpose?
                                                        // Use SignalN or SignalNR instead.
            ASSERT_DEBUG_ONLY(delegate.callable());     // Delegate should be callable..
            if( delegate.callable() )                   // ..and, in release we don't want to call it.
                _m_delegate = delegate;

            // we do not return an id since a Signal cannot have more than one connection (see Broadcast_Signal).
        }

        void disconnect()
        {
            _m_delegate = {};
            ASSERT_DEBUG_ONLY(_m_delegate.is_null());
        }

        Result_Type emit(Arg_Types...args) const // emit signal to the listener, if any.
        {
            _m_delegate.call(args...); // calling a null delegate has no effect
        }

    private:
        Delegate_Type _m_delegate;
    };
    
    //
    // Similar to SignalR, but can be connected to multiple Delegates
    //
    template<typename Result_Type, typename ...Args_Types>
    struct Broadcast_Signal<Result_Type(Args_Types...)>
    {
        using Delegate_Type = Delegate<Result_Type(Args_Types...)>; // the expected delegate type to connect with

        Broadcast_Signal() = default;
        Broadcast_Signal(const Broadcast_Signal&) = delete;

        // TMethod: the address to a member function
        // object_ptr: the instance to call the member with
        // Inserting multiple times the same method for the same pointer is undefined behavior.
        template<auto Method_Type>
        requires std::is_member_function_pointer_v<decltype(Method_Type)>
        void connect(void* object_ptr)
        {
            auto delegate = Delegate_Type::template from_method<Method_Type>(object_ptr);
            VERIFY( delegate.callable(), "TMethod is not callable on object_ptr. Is object_ptr null?" );
            connect(delegate);
            // TODO: return an identifier/hash to disconnect with?
        }

        void connect(const Delegate_Type& delegate)
        {
            _m_delegate.emplace_back(std::move(delegate));
        }

        template<auto Method_Type>
        requires std::is_member_function_pointer_v<decltype(Method_Type)>
        bool disconnect(void* ptr) // Disconnects a given (TMethod, ptr) delegate
        {
            // We assume there is only one delegate per (TMethod, ptr) pair
            auto delegate = Delegate_Type::template from_method<Method_Type>(ptr);
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

        Result_Type broadcast(Args_Types...args) const // broadcast to all the listeners
        {
            for(const auto& delegate : _m_delegate )
                delegate.call(args...);
        }

    private:
        std::vector<Delegate_Type> _m_delegate;
    };
}
