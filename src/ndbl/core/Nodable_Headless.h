#pragma once

#include "bdc/String.hpp"
#include "bdc/String_Builder.hpp"

namespace tools
{
    struct Task_Manager;
}

namespace ndbl
{
    using bdc::String_Builder;
    using bdc::String;

    // forward declarations
    struct Graph;
    class Language;

    struct App_Headless_State
    {
        bool    auto_completion;
        bool    should_stop;
        Graph*  graph;
        String  source_code;
    };

    void            nodable_init(App_Headless_State*);
    void            nodable_deinit(App_Headless_State*);
    void            nodable_update(App_Headless_State*);
    void            nodable_clear(App_Headless_State*);
    String_Builder& nodable_serialize(const App_Headless_State*, String_Builder& out);
    Graph*          nodable_parse(const App_Headless_State*, const String& in );
}

