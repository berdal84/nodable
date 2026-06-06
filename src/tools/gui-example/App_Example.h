#pragma once
#include "tools/gui/App.h"
#include "tools/gui/Config.h"
#include "App_View_Example.h"

namespace tools
{
    // forward declaration
    class App_View_Example;
    
    class App_Example : public App_State
    {
        friend App_View_Example;
    public:
        void        init();
        void        run();
        void        shutdown();
        void        update();
        void        draw();
        bool        should_stop() const { return app_should_stop(this); };
        void        _do_frame();
    private:
        void        request_stop();

        App_View_Example  m_view;
        Config*         m_config = nullptr;
    };
}