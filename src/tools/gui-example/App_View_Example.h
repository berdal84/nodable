#pragma once
#include "tools/gui/App_View.h"

namespace tools
{
    // Forward declarations
    class App_Example;

    // This example is OOP
    class App_View_Example : public App_View_State
    {
    public:
        void            init(App_Example* _app);
        void            shutdown();
        void            update();
        void            draw();
        void            _draw_splashscreen_content();
        void            _reset_layout();
    private:
        App_Example*     m_app = nullptr; // NOT owned
    };
}