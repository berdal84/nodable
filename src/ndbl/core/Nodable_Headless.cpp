#include "Nodable_Headless.h"
#include "core/Graph.h"
#include "ndbl/core/language/Nodlang.h"
#include "tools/core/Task_Manager.h"

using namespace ndbl;

void ndbl::nodable_init(App_Headless_State* state)
{
    // init managers
    state->task_manager = tools::init_task_manager();
    state->language     = init_language();

    // configure
    state->graph = new Graph();
    graph_init(state->graph);
    state->language->_state.reset( state->graph ); // in some cases (like during tests), we call parse_xxx methods that implicitly requires the state to be reset
}

void ndbl::nodable_deinit(App_Headless_State* state)
{
    ASSERT(state->graph);
    nodable_clear(state);
    graph_deinit(state->graph);
    delete state->graph;
    tools::shutdown_task_manager(state->task_manager);
    shutdown_language(state->language);
}

std::string& ndbl::nodable_serialize(const App_Headless_State* state, std::string& out )
{
    return state->language->serialize_graph(out, state->graph);
}

Graph* ndbl::nodable_parse(const App_Headless_State* state,  const std::string& code )
{
    state->language->parse( state->graph, code );
    return state->graph;
}

void ndbl::nodable_update(App_Headless_State* state)
{
    //
    // nothing is required there for now.
    //
}

void ndbl::nodable_clear(App_Headless_State* state)
{
    graph_reset(state->graph);
    state->source_code.clear();
}
