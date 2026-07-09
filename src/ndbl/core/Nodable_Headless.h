#pragma once

#include <string>

namespace tools
{
    struct Task_Manager;
}

namespace ndbl
{
    // forward declarations
    struct Graph;
    class Nodlang;

    struct App_Headless_State
    {
        bool                    auto_completion = false;
        bool                    should_stop     = false;
        tools::Task_Manager*    task_manager    = nullptr; // ref
        Graph*                  graph           = nullptr; // ref
        Nodlang*                language        = nullptr; // ref
        std::string             source_code     = "";
    };

    void            nodable_init(App_Headless_State*);
    void            nodable_update(App_Headless_State*);
    void            nodable_shutdown(App_Headless_State*);
    void            nodable_clear(App_Headless_State*);
    std::string&    nodable_serialize(const App_Headless_State*, std::string& out);
    Graph*          nodable_parse(const App_Headless_State*, const std::string& in );
}

