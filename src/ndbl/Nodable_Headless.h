#pragma once
#include "bdc/String.hpp"

namespace ndbl
{
    // forward declarations
    struct Graph;

    struct App_Headless_State
    {
        bool        auto_completion;
        bool        should_stop;
        Graph*      graph;
        bdc::String source_code;
    };

    void            nodable_init(App_Headless_State*);
    void            nodable_deinit(App_Headless_State*);
    void            nodable_update(App_Headless_State*);
    void            nodable_clear(App_Headless_State*);
    bdc::String     nodable_serialize(const App_Headless_State*);
    Graph*          nodable_parse(const App_Headless_State*, const bdc::String& in );
}

