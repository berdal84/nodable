#include "StateMachine.h"
#include "tools/core/assertions.h"

using namespace tools;

StateMachine::~StateMachine()
{
    for(auto& [hash, state] : m_state)
        delete state;
}

void StateMachine::set_default_state(const char* name)
{
    State* state = get_state(name);
    EXPECT(state != nullptr, "Can't find state")
    EXPECT(m_default_state == nullptr, "This method must be called once")
    m_default_state = state;
}

void StateMachine::tick()
{
    if ( !m_started )
        return;

#if TOOLS_DEBUG_STATE_MACHINE
    if ( ImGui::Begin("StateMachine Debug"))
    {
        ImGui::Text("Current State:     %s (id %u)", m_current_state->name, m_current_state->id);
        ImGui::End();
    }
#endif

    ASSERT(m_current_state != nullptr)
    m_current_state->tick(m_context_ptr);

    // Early return if no transition is found
    if ( m_next_state == nullptr )
        return;

    // Switch to next_state
    m_current_state->leave(m_context_ptr);
    m_current_state = m_next_state;
    m_current_state->enter(m_context_ptr);
    m_current_state->post_tick(m_context_ptr); // TODO: this is a relic of the past, remove it
    m_next_state = nullptr;
}

void StateMachine::start()
{
    EXPECT(m_started == false, "StateMachine is already started")
    m_started = true;
    m_current_state = m_default_state;
}

void StateMachine::stop()
{
    EXPECT(m_started == true, "StateMachine is not started")
    m_started = false;
}

State* StateMachine::add_state(const char *name)
{
    auto* state = new State(name);
    add_state(state);
    return state;
}

void StateMachine::add_state(State* state)
{
    u32_t key = hash::hash(state->name);
    EXPECT(m_state.find(key) == m_state.end(), "State name already exists")
    m_state.insert({key, state});
}

void StateMachine::change_state(State* state)
{
    EXPECT(m_next_state == nullptr, "Can't change twice within a single tick")
    m_next_state = state;
}

void StateMachine::exit_state()
{
    EXPECT(m_current_state != m_default_state, "Default state can't be exited!")
    change_state(m_default_state);
}

State *StateMachine::get_state(const char *name)
{
    auto it = m_state.find( hash::hash(name) );
    if( it != m_state.end())
        return it->second;
    return nullptr;
}

void StateMachine::change_state(const char *name)
{
    State* state = get_state(name);
    EXPECT(state != nullptr, "Unable to find state")
    change_state(state);
}