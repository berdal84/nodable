
#include "Config.h"
#include "fixtures/gui.h"
#include "tools/gui/Config.h"
#include "tools/gui/gui.h"

typedef ::testing::Gui Gui_App;
using namespace ndbl;

TEST_F(Gui_App, constructor)
{
    EXPECT_NO_THROW(Nodable app);
}

TEST_F(Gui_App, get_instance)
{
    {
        EXPECT_ANY_THROW(Nodable::get_instance());
        Nodable app;
        EXPECT_NO_THROW(Nodable::get_instance());
    }
    EXPECT_ANY_THROW(Nodable::get_instance());
}

TEST_F(Gui_App, init)
{
    Nodable app;
    EXPECT_NO_THROW(app.init());
    EXPECT_TRUE(app.shutdown());
}

TEST_F(Gui_App, shutdown)
{
    Nodable app;
    EXPECT_TRUE(app.init());
    EXPECT_NO_THROW(app.shutdown());
}

TEST_F(Gui_App, update)
{
    Nodable app;
    EXPECT_TRUE(app.init());
    EXPECT_NO_THROW(app.update());
    EXPECT_TRUE(app.shutdown());
}

TEST_F(Gui_App, loop_count_1)
{
    Nodable app;
    EXPECT_TRUE(app.init());
    loop_count(app, 1);
    EXPECT_TRUE(app.shutdown());
}

TEST_F(Gui_App, loop_duration_5s)
{
    Nodable app;
    EXPECT_TRUE(app.init());
    loop_duration(app, 5.0);
    EXPECT_TRUE(app.shutdown());
}

TEST_F(Gui_App, new_file)
{
    Nodable app;
    app.init();
    app.show_splashscreen(false);
    app.new_file();
    loop_duration(app, 1.0);
    save_screenshot(app, "TEST_Gui_App__new_file__0.png");
    app.shutdown();
}

TEST_F(Gui_App, open_file)
{
    Nodable app;
    app.init();
    app.show_splashscreen(false);
    loop_duration(app, 1.0);
    save_screenshot(app, "TEST_Gui_App__open_file__0.png");
    EXPECT_TRUE(app.open_asset_file("./examples/arithmetic.cpp"));
    loop_duration(app, 1.0);
    save_screenshot(app, "TEST_Gui_App__open_file__1.png");
    app.shutdown();
}

TEST_F(Gui_App, close_file)
{
    Nodable app;
    app.init();
    app.show_splashscreen(false);
    File* file = app.open_asset_file("./examples/arithmetic.cpp");
    loop_duration(app, 1.0);
    save_screenshot(app, "TEST_Gui_App__close_file__0.png");
    app.close_file( file );
    loop_duration(app, 1.0);
    save_screenshot(app, "TEST_Gui_App__close_file__1.png");
    app.shutdown();
}

TEST_F(Gui_App, open_examples)
{
    Nodable app;
    app.init();
    app.show_splashscreen(false);
    app.update();
    save_screenshot(app, "TEST_Gui_App__open_examples__0.png");
    EXPECT_TRUE(app.open_asset_file("./examples/arithmetic.cpp"));
    loop_duration(app, 1.0);
    save_screenshot(app, "TEST_Gui_App__open_examples__1.png");
    EXPECT_TRUE(app.open_asset_file("./examples/for-loop.cpp"));
    loop_duration(app, 1.0);
    save_screenshot(app, "TEST_Gui_App__open_examples__2.png");
    EXPECT_TRUE(app.open_asset_file("./examples/if-else.cpp"));
    loop_duration(app, 1.0);
    save_screenshot(app, "TEST_Gui_App__open_examples__3.png");
    EXPECT_TRUE(app.open_asset_file("./examples/multi-instructions.cpp"));
    loop_duration(app, 1.0);
    save_screenshot(app, "TEST_Gui_App__open_examples__4.png");
    app.shutdown();
}



