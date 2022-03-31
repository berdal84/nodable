#pragma once

namespace Nodable
{
    // forward decl
    class App;
    namespace vm
    {
        class VM;
    }
    class Settings;
    class Language;
    class TextureManager;

    /**
     * @brief Simple structure to store application context.
     * (in order to avoid singleton I use a context instance I pass to the application and to any instance requiring it)
     */
    class AppContext
    {
    friend class App;
    public:
        AppContext(App* _app)
            : settings(nullptr)
            , app(_app)
            , elapsed_time(0)
            , language(nullptr)
            , vm(nullptr)
            , texture_manager(nullptr)
            {}

        ~AppContext();

        Settings*       settings;
        App*            app;
        vm::VM*         vm;
        const Language* language;
        TextureManager* texture_manager;
        float           elapsed_time;
    private:
        static AppContext* create_default(App* _app);
    };


}