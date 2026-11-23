#pragma once

#include "bdc/String.hpp"
#include "Parser.h"

namespace ndbl
{
    // forward declarations
    struct Graph;

    struct App_Headless_State
    {
        bool                auto_completion;
        bool                should_stop;
        Graph*              graph;
        bdc::String         source_code;
        Parser_Context      parser;
        Serializer_Context  serializer;
    };

    void            nodable_init(App_Headless_State*);
    void            nodable_deinit(App_Headless_State*);
    void            nodable_update(App_Headless_State*);
    void            nodable_clear(App_Headless_State*);
    bdc::String     nodable_serialize(App_Headless_State*);
    Graph*          nodable_parse(App_Headless_State*, const bdc::String& in );
}

