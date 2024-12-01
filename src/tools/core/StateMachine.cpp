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
    VERIFY(state != nullptr, "Can't find state");
    VERIFY(m_default_state == nullptr, "This method must be called once");
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

    ASSERT(m_current_state != nullptr);
    m_current_state->delegate[OnTick].call();

    // Early return if no transition is found
    if ( m_next_state == nullptr )
        return;

    // Switch to next_state
    m_current_state->delegate[OnLeave].call();
    m_current_state = m_next_state;
    m_current_state->delegate[OnEnter].call();
    m_next_state = nullptr;
}

void StateMachine::start()
{
    VERIFY(m_started == false, "StateMachine is already started");
    m_started = true;
    m_current_state = m_default_state;
}

void StateMachine::stop()
{
    VERIFY(m_started == true, "StateMachine is not started");
    m_started = false;
}

State* StateMachine::add_state(const char* _name)
{
    auto* state = new State();
    state->name = _name;
    add_state(state);
    return state;
}

void StateMachine::add_state(State* state)
{
    const u32_t key = Hash::hash32( state->name );
    VERIFY(m_state.find(key) == m_state.end(), "State name already exists");
    m_state.insert({key, state});
}

void StateMachine::change_state(State* state)
{
    VERIFY(m_next_state == nullptr, "Can't change twice within a single tick");
    m_next_state = state;
}

void StateMachine::exit_state()
{
    VERIFY(m_current_state != m_default_state, "Default state can't be exited!");
    change_state(m_default_state);
}

State *StateMachine::get_state(const char *name)
{
    auto it = m_state.find( Hash::hash32(name) );
    if( it != m_state.end())
        return it->second;
    return nullptr;
}

void StateMachine::change_state(const char *name)
{
    State* state = get_state(name);
    VERIFY(state != nullptr, "Unable to find state");
    change_state(state);
}