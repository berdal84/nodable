#include "StateMachine.h"
#include "tools/core/assertions.h"

using namespace tools;

void StateMachine::add_transition(Transition* transition)
{
    for(Transition* each : m_transitions)
        ASSERT(each != transition); // already present (should we use std::map ? add_transition happens few times at the begining only)
    m_transitions.emplace_back(transition);
}

void StateMachine::set_default_state(State* default_state)
{
    EXPECT(m_current_state == nullptr, "called twice?")
    m_current_state = default_state;
    EXPECT(m_current_state != nullptr, "StateMachine must have a valid default_state")
}

void StateMachine::tick()
{
    if ( !m_started )
        return;

    ASSERT(m_current_state != nullptr)
    m_current_state->on_tick();

    // Find the next transition to apply
    const Transition* transition_to_apply = nullptr;
    for(const Transition* transition : m_transitions )
    {
        if ( transition->should_apply(m_current_state) )
        {
            transition_to_apply = transition;
            break;
        }
    }

    // Early return if no transition is found
    if ( transition_to_apply == nullptr )
        return;

    // Leave current state
    m_current_state->on_leave();
    delete m_current_state;

    // Enter next state
    State* next_state = transition_to_apply->apply(m_current_state);
    ASSERT(next_state != nullptr)
    next_state->on_enter();
    m_current_state = next_state;
}

void StateMachine::start()
{
    ASSERT(m_started == false)
    m_started = true;
}

void StateMachine::stop()
{
    ASSERT(m_started == true)
    m_started = false;
}
