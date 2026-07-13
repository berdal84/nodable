#pragma once
#include <unordered_map>
#include "Types.h"
#include "tools/core/Asserts.h"
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
        const char*      name{};
        Simple_Delegate   delegate[When_COUNT]{};
    };

    class State_Machine
    {
    public:
        State_Machine(void* context_ptr): m_context_ptr(context_ptr) {};
        ~State_Machine();

        void         start();
        void         tick();
        void         stop();
        void         set_default_state(const char* name);
        const char*  get_current_state_name() const { return m_current_state->name; }
        bool         has_default_state() const { return m_current_state == m_default_state; }
        State*       add_state(const char* name);
        
        template<auto Function>
        void bind(const char* name, When when)
        {
            // Guards
            VERIFY( when < When_COUNT, "when argument is out of bound" );

            // Find the state
            State* state = get_state(name);
            ASSERT(state != nullptr);

            // Override the delegate
            state->delegate[when] = Simple_Delegate::from<Function>(m_context_ptr);
        }

        void change_state(const char* name);
        void exit_state();

    private:
        bool   started() const { return m_current_state != nullptr; }
        void   add_state(State*);
        State* get_state(const char* name);
        void   set_next_state(State *state);

        void*  m_context_ptr;
        State* m_default_state = nullptr;
        State* m_current_state = nullptr;
        State* m_next_state    = nullptr;

        struct No_Hash
        {
            constexpr u64_t operator()(u64_t u) const // pass through
            { return u; }
        };

        std::unordered_map<u64_t, State*, No_Hash> m_state; // u32_t is state.name hash
    };

} // namespace tools