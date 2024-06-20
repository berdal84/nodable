#pragma once
#include <functional>
#include "types.h"

namespace tools
{
    class State
    {
    public:
        State() = delete;
        State(u32_t id): id(id) {}
        virtual void on_enter() {};
        virtual void on_tick()  {};
        virtual void on_leave() {};
        const u32_t  id;
    };

    struct Transition
    {
        using Condition = std::function<bool (const State* current_state )> ;
        using Factory   = std::function<State*(State* current_state )> ;

        Transition(Condition&& condition, Factory&& factory)
        : m_condition(condition)
        , m_factory(factory)
        {}

        inline bool   should_apply(const State* curr_state) const
        { return m_condition(curr_state); }

        inline State* apply(State* curr_state) const
        { return m_factory(curr_state); };

        const Condition m_condition;
        const Factory   m_factory;
    };

    // Base template to declare a StateMachine
    class StateMachine
    {
    public:
        StateMachine() = default;
        ~StateMachine() = default;

        void start();
        virtual void tick();
        void stop();
        void add_transition(Transition*); // Add a transition to the list (processed in FIFO)
        void set_default_state(State* default_state);
        State* get_current_state() { return m_current_state; }
        const State* get_current_state() const { return m_current_state; }
    protected:
        bool   m_started{false};
        State* m_current_state{nullptr}; // Data pointed is NOT owned by this class
        std::vector<Transition*> m_transitions{};
    };
}