#pragma once
#include "AppExampleView.h"
#include "tools/gui/BaseApp.h"
#include "tools/gui/Config.h"

namespace tools
{
    class AppExample : public tools::BaseApp
    {
    public:
        AppExample() = default;
        ~AppExample() override = default;

        void init()
        {
            LOG_MESSAGE("AppExample", "init() ...\n");

            BaseApp::init(new AppExampleView(this));
            this->view->set_title("framework-example - (based on framework-gui library)");

            LOG_MESSAGE("AppExample", "init() DONE\n");
        }

        void shutdown()
        {
            LOG_MESSAGE("AppExample", "shutdown() ...\n");

            BaseApp::shutdown();
            delete view;

            LOG_MESSAGE("AppExample", "shutdown() DONE\n");
        }
    };
}