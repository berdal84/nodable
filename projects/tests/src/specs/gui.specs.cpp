#include "../fixtures/gui.h"

typedef ::testing::Gui Gui_App;

TEST_F(Gui_App, constructor)
{
    EXPECT_NO_THROW(ndbl::App app);
}

TEST_F(Gui_App, get_instance)
{
    ndbl::App  app;
    EXPECT_NO_THROW(ndbl::App::get_instance());
}

TEST_F(Gui_App, init)
{
    ndbl::App app;
    EXPECT_NO_THROW(app.init());
    EXPECT_TRUE(app.shutdown());
}

TEST_F(Gui_App, shutdown)
{
    ndbl::App app;
    EXPECT_TRUE(app.init());
    EXPECT_NO_THROW(app.shutdown());
}

TEST_F(Gui_App, update)
{
    ndbl::App app;
    EXPECT_TRUE(app.init());
    EXPECT_NO_THROW(app.update());
    EXPECT_TRUE(app.shutdown());
}

TEST_F(Gui_App, loop_count_1)
{
    ndbl::App app;
    EXPECT_TRUE(app.init());
    loop_count(app, 1);
    EXPECT_TRUE(app.shutdown());
}

TEST_F(Gui_App, loop_duration_5s)
{
    ndbl::App app;
    EXPECT_TRUE(app.init());
    loop_duration(app, 5.0);
    EXPECT_TRUE(app.shutdown());
}

TEST_F(Gui_App, open_file)
{
    ndbl::App app;
    app.init();
    app.get_view()->set_splashscreen_visible(false);
    app.update();
    EXPECT_TRUE(app.open_file("./examples/arithmetic.cpp", true));
    app.shutdown();
}

TEST_F(Gui_App, close_file)
{
    ndbl::App app;
    app.init();
    app.get_view()->set_splashscreen_visible(false);
    app.update();
    app.open_file("./examples/arithmetic.cpp", true);
    loop_duration(app, 1.0);
    app.close_file( app.current_file() );
    loop_duration(app, 1.0);
    app.shutdown();
}

TEST_F(Gui_App, open_examples)
{
    ndbl::App app;
    app.init();
    app.get_view()->set_splashscreen_visible(false);
    app.update();
    EXPECT_TRUE(app.open_file("./examples/arithmetic.cpp", true));
    loop_duration(app, 0.5);
    EXPECT_TRUE(app.open_file("./examples/for-loop.cpp", true));
    loop_duration(app, 0.5);
    EXPECT_TRUE(app.open_file("./examples/if-else.cpp", true));
    loop_duration(app, 0.5);
    EXPECT_TRUE(app.open_file("./examples/multi-instructions.cpp", true));
    loop_duration(app, 1.0);
    app.shutdown();
}



