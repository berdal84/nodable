#pragma once
#include "AppExampleView.h"
#include "tools/gui/BaseApp.h"
#include "tools/gui/Config.h"

namespace tools
{
    class AppExample : public tools::BaseApp
    {
    public:
        AppExample()
        : BaseApp(new AppExampleView(this))
        {}

        ~AppExample()
        {
            delete view;
        }

        void init() override
        {
            LOG_MESSAGE("AppExample", "init() ...\n");
            BaseApp::init();
            view->set_title("framework-example - (based on framework-gui library)");

            // custom code here

            LOG_MESSAGE("AppExample", "init() DONE\n");
        }

        void shutdown() override
        {
            LOG_MESSAGE("AppExample", "shutdown() ...\n");

            // custom code here

            BaseApp::shutdown();
            LOG_MESSAGE("AppExample", "shutdown() DONE\n");
        }
    };
}