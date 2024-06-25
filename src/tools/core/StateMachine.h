#pragma once
#include <functional>
#include "types.h"
#include "tools/core/reflection/function_traits.h"
#include "tools/core/assertions.h"
#include "tools/core/hash.h"

#ifdef TOOLS_DEBUG
#define TOOLS_DEBUG_STATE_MACHINE 0
#endif

namespace tools
{

    struct State
    {
        // Wrapped member function pointer, can be called for a given instance_ptr
        typedef std::function<void(void* context_ptr)> Delegate; // TODO: perf, avoid using std::function

        State(const char* name)
        :  name(name)
        {}

        const char*   name;
        Delegate      enter     = [](void*) {};
        Delegate      tick      = [](void*) {};
        Delegate      post_tick = [](void*) {};
        Delegate      leave     = [](void*) {};
    };

    // from a current state, return a new state or nullptr
    //typedef std::function<State*(void* instance_ptr, const State* curr_state)> Transition; // TODO: perf, avoid using std::function

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

        template<typename TMember>
        void bind_enter(const char* name, TMember member_ptr);

        template<typename TMember>
        void bind_tick(const char* name, TMember member_ptr);

        template<typename TMember>
        void bind_post_tick(const char* name, TMember member_ptr);

        template<typename TMember>
        void bind_leave(const char* name, TMember member_ptr);

        void change_state(const char* name);
        void exit_state();

// Not necessary until we need a state hierachy
//        template<typename TMember>
//        void add_transition(State* state, TMember member_ptr);

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
        // std::vector<Transition> m_transitions{};
    };

#define DECLARE_BIND( DELEGATE_NAME ) \
    template<typename TMember>\
    void StateMachine::bind_##DELEGATE_NAME(const char* name, TMember member_ptr)\
    {\
        using T = typename FunctionTrait<TMember>::class_t;\
        State* state = get_state(name);\
        state->DELEGATE_NAME = [member_ptr](void* context_ptr) {\
            return (((T*)context_ptr)->*member_ptr)();\
        };\
    }

    DECLARE_BIND(enter)
    DECLARE_BIND(tick)
    DECLARE_BIND(post_tick)
    DECLARE_BIND(leave)

#undef DECLARE_BIND


// Not necessary until we need a state hierachy
//    template<typename TMember>
//    void StateMachine::add_transition(State *state, TMember member_ptr)
//    {
//        using T = typename FunctionTrait<TMember>::class_t;
//        auto transition = [member_ptr](void* context_ptr, const State* current_state) -> State*
//        {
//            T* instance_ptr = ((T*)context_ptr);
//            return (instance_ptr->*member_ptr)(current_state);
//        };
//        m_transitions.emplace_back(transition);
//    }

} // namespace tools