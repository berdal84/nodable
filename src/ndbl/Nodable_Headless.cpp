#include "Nodable_Headless.h"

#include "bdc/String_Builder.hpp"
#include "Graph.h"
#include "Parser.h"
#include "Task_Manager.h"
#include "reflection/index.h"

using namespace ndbl;

void ndbl::nodable_init(App_Headless_State* state)
{
    // init managers
    reflection_init();
    memory_manager_init();
    task_manager_init();
    language_init();

    // configure
    state->graph = bdc::memory_new<Graph>();
    graph_init(state->graph);
    lang_reset( parser(), state->graph ); // in some cases (like during tests), we call parse_xxx methods that implicitly requires the state to be reset
}

void ndbl::nodable_deinit(App_Headless_State* state)
{
    ASSERT(state->graph);
    nodable_clear(state);
    graph_deinit(state->graph);
    bdc::memory_delete(state->graph);
    task_manager_shutdown();
    language_shutdown();
    memory_manager_shutdown();
    reflection_shutdown();
}

bdc::String ndbl::nodable_serialize(const App_Headless_State* state )
{
    String_Builder sb;
    string_builder_init(sb);
    lang_serialize_graph(parser(), sb, state->graph);
    return string_builder_build_tstring(sb);
}

Graph* ndbl::nodable_parse(const App_Headless_State* state,  const bdc::String& in_code )
{
    if( !lang_parse(parser(), state->graph, in_code ) )
        return nullptr;
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
    string_reset( state->source_code );
}
