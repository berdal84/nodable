#pragma once
#include <functional>
#include <unordered_map>
#include "types.h"
#include "tools/core/assertions.h"
#include "tools/core/hash.h"
#include "tools/core/Delegate.h"

#ifdef TOOLS_DEBUG
#define TOOLS_DEBUG_STATE_MACHINE 0
#endif

namespace tools
{
    enum When : u8_t
    {
        OnEnter    = 0,
        OnTick     = 1,
        OnLeave    = 2,
        When_COUNT
    };

    //
    // struct State is essentially holding 3 delegates and a name.
    // Delegates are indexed with an "enum When", ex: State state{}; state.delegate[OnTick].call()
    //
    struct State
    {
        const char*      name  = nullptr;
        Delegate_NoArgs  delegate[When_COUNT];
    };

    class StateMachine
    {
    public:
        StateMachine(void* context_ptr): m_context_ptr(context_ptr) {};
        ~StateMachine();

        void         start();
        void         tick();
        void         stop();
        void         set_default_state(const char* name);
        const char*  get_current_state_name() const { return m_current_state->name; }
        bool         has_default_state() const { return m_current_state == m_default_state; }
        State*       add_state(const char* name);
        template<auto TMethod> void bind(const char* name, When when)
        {
            // Guards
            VERIFY( when < When_COUNT, "when argument is out of bound" );

            // Find the state
            State* state = get_state(name);
            ASSERT(state != nullptr);

            // Override the delegate
            state->delegate[when] = Delegate_NoArgs::from_method<TMethod>(m_context_ptr);
        }

        void change_state(const char* name);
        void exit_state();

    private:
        void   add_state(State*);
        State* get_state(const char* name);
        void   change_state(State*);

        void*  m_context_ptr;
        bool   m_started       = false;
        State* m_default_state = nullptr;
        State* m_current_state = nullptr;
        State* m_next_state    = nullptr;
        std::unordered_map<u32_t, State*> m_state; // u32_t is state.name hash
    };

} // namespace tools