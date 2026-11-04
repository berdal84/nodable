#include "State_Machine.h"

#include "Asserts.h"
#include "Hash.h"

using namespace ndbl;

void State_Machine::set_default_state(const char* name)
{
    State* state = get_state(name);
    NDBL_VERIFY(state != nullptr, "Can't find state");
    NDBL_VERIFY(m_default_state == nullptr, "This method must be called once");
    m_default_state = state;
}

void State_Machine::tick()
{
    if ( !started() )
        return;

#if NDBL_DEBUG_STATE_MACHINE
    if ( ImGui::Begin("State_Machine Debug"))
    {
        ImGui::Text("Current State:     %s (id %u)", m_current_state->name, m_current_state->id);
        ImGui::End();
    }
#endif

    NDBL_ASSERT(m_current_state != nullptr);
    m_current_state->delegate[OnTick].call();

    // Early return if no transition is found
    if ( m_next_state == nullptr )
        return;

    // Switch to next_state
    m_current_state->delegate[OnLeave].call();
    m_next_state->delegate[OnEnter].call();
    m_current_state = m_next_state;
    m_next_state    = nullptr;
}

void State_Machine::start()
{
    NDBL_VERIFY( m_context_ptr != nullptr, "Did you call init(void* context) first?");
    NDBL_VERIFY( !started(), "State_Machine is already started");
    m_current_state = m_default_state;
    m_current_state->delegate[OnEnter].call();
}

void State_Machine::stop()
{
    NDBL_VERIFY( started(), "State_Machine is not started");
    m_current_state->delegate[OnLeave].call();
    m_current_state = nullptr;
}

State* State_Machine::create_state(const char* _name)
{
    const auto key = Hash::hash( _name );
    const auto& [it, success] = m_state.emplace( key, State{ .name = _name});

    NDBL_VERIFY(success, "State name already exists");
    
    return &it->second;
}

void State_Machine::set_next_state(State* state)
{
    NDBL_VERIFY(m_next_state == nullptr, "Can't change twice within a single tick");
    m_next_state = state;
}

void State_Machine::exit_state()
{
    NDBL_VERIFY(m_current_state != m_default_state, "Default state can't be exited!");
    set_next_state(m_default_state);
}

State* State_Machine::get_state(const char *name)
{
    auto it = m_state.find( Hash::hash(name) );
    if( it != m_state.end())
        return &it->second;
    return nullptr;
}

void State_Machine::change_state(const char *name)
{
    State* state = get_state(name);
    NDBL_VERIFY(state != nullptr, "Unable to find state");
    set_next_state(state);
}