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
    langdef_init();
    parser_init(state->parser);

    // configure
    state->graph = bdc::memory_new<Graph>();
    graph_init(state->graph);
    parser_reset( state->parser, state->graph ); // in some cases (like during tests), we call parse_xxx methods that implicitly requires the state to be reset
}

void ndbl::nodable_deinit(App_Headless_State* state)
{
    ASSERT(state->graph);
    nodable_clear(state);
    graph_deinit(state->graph);
    bdc::memory_delete(state->graph);
    task_manager_shutdown();
    langdef_shutdown();
    memory_manager_shutdown();
    reflection_shutdown();
}

bdc::String ndbl::nodable_serialize(App_Headless_State* state )
{
    serialize_graph(state->parser, state->graph);
    return parser_build_string(state->parser);
}

Graph* ndbl::nodable_parse(App_Headless_State* state,  const bdc::String& str )
{
    if( !parse(state->parser, state->graph, str ) )
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
