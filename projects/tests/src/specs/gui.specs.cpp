#include "../fixtures/gui.h"

typedef ::testing::Gui Gui_App;

TEST_F(Gui_App, constructor)
{
    EXPECT_NO_THROW(ndbl::App app);
}

TEST_F(Gui_App, get_instance)
{
    {
        EXPECT_ANY_THROW(ndbl::App::get_instance());
        ndbl::App  app;
        EXPECT_NO_THROW(ndbl::App::get_instance());
    }
    EXPECT_ANY_THROW(ndbl::App::get_instance());
}

TEST_F(Gui_App, init)
{
    fw::log::set_verbosity(fw::log::Verbosity_Verbose);
    ndbl::App app;
    EXPECT_NO_THROW(app.framework.init());
    EXPECT_TRUE(app.framework.shutdown());
}

TEST_F(Gui_App, shutdown)
{
    ndbl::App app;
    EXPECT_TRUE(app.framework.init());
    EXPECT_NO_THROW(app.framework.shutdown());
}

TEST_F(Gui_App, update)
{
    ndbl::App app;
    EXPECT_TRUE(app.framework.init());
    EXPECT_NO_THROW(app.framework.update());
    EXPECT_TRUE(app.framework.shutdown());
}

TEST_F(Gui_App, loop_count_1)
{
    ndbl::App app;
    EXPECT_TRUE(app.framework.init());
    loop_count(app, 1);
    EXPECT_TRUE(app.framework.shutdown());
}

TEST_F(Gui_App, loop_duration_5s)
{
    ndbl::App app;
    EXPECT_TRUE(app.framework.init());
    loop_duration(app, 5.0);
    EXPECT_TRUE(app.framework.shutdown());
}

TEST_F(Gui_App, open_file)
{
    ndbl::App app;
    app.framework.init();
    app.framework.config.splashscreen = false;
    loop_duration(app, 1.0);
    save_screenshot(app, "TEST_Gui_App__open_file__0.png");
    EXPECT_TRUE(app.open_file("./examples/arithmetic.cpp"));
    loop_duration(app, 1.0);
    save_screenshot(app, "TEST_Gui_App__open_file__1.png");
    app.framework.shutdown();
}

TEST_F(Gui_App, close_file)
{
    ndbl::App app;
    app.framework.init();
    app.framework.config.splashscreen = false;
    ndbl::File* file = app.open_file("./examples/arithmetic.cpp");
    loop_duration(app, 1.0);
    save_screenshot(app, "TEST_Gui_App__close_file__0.png");
    app.close_file( file );
    loop_duration(app, 1.0);
    save_screenshot(app, "TEST_Gui_App__close_file__1.png");
    app.framework.shutdown();
}

TEST_F(Gui_App, open_examples)
{
    ndbl::App app;
    app.framework.init();
    app.framework.config.splashscreen = false;
    app.framework.update();
    save_screenshot(app, "TEST_Gui_App__open_examples__0.png");
    EXPECT_TRUE(app.open_file("./examples/arithmetic.cpp"));
    loop_duration(app, 1.0);
    save_screenshot(app, "TEST_Gui_App__open_examples__1.png");
    EXPECT_TRUE(app.open_file("./examples/for-loop.cpp"));
    loop_duration(app, 1.0);
    save_screenshot(app, "TEST_Gui_App__open_examples__2.png");
    EXPECT_TRUE(app.open_file("./examples/if-else.cpp"));
    loop_duration(app, 1.0);
    save_screenshot(app, "TEST_Gui_App__open_examples__3.png");
    EXPECT_TRUE(app.open_file("./examples/multi-instructions.cpp"));
    loop_duration(app, 1.0);
    save_screenshot(app, "TEST_Gui_App__open_examples__4.png");
    app.framework.shutdown();
}



