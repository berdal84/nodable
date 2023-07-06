#include <gtest/gtest.h>

#include "nodable/gui/Nodable.h"
#include <thread>

#ifdef NDBL_GUI_TEST_HUMAN_SPEED
#define SLEEP_FOR_HUMAN( delay_in_ms ) std::this_thread::sleep_for( std::chrono::milliseconds( delay_in_ms ) );
#else
#define SLEEP_FOR_HUMAN( delay_in_ms )
#endif

namespace testing
{
    class Gui: public Test {
    public:
        Gui()
        {
            // Override app label with test name
            const TestInfo* test_info = ::testing::UnitTest::GetInstance()->current_test_info();
            std::string label = "Test | ";
            label += test_info->test_suite_name();
            label += " - ";
            label += test_info->name();
            // ndbl::Settings::get_instance().fw_app_view.app_window_label = label;
        }

        /**
         * run some loops for a given iteration count
         */
        static void loop_count(ndbl::Nodable & app, size_t iteration_count, double sleep_in_sec = 1.0)
        {
            for(int i = 0; i < iteration_count; ++i)
            {
                EXPECT_NO_THROW(app.framework.update());
                EXPECT_NO_THROW(app.framework.draw());
            }
            SLEEP_FOR_HUMAN((long)(1000.0 * sleep_in_sec));
        }

        /**
         * run some loops for a given duration
         */
        static void loop_duration(ndbl::Nodable & app, double duration_in_sec)
        {
            auto start = std::chrono::system_clock::now();
            auto end   = std::chrono::system_clock::now();
            size_t iteration = 0;
            while((std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() < (long long)(duration_in_sec * 1000.f)))
            {
                end = std::chrono::system_clock::now();
                LOG_MESSAGE("Test", "Loop iteration %llu (time: %0.1f/%0.1f sec)\n"
                , iteration
                , float(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()) / 1000.0f
                , duration_in_sec);
                EXPECT_NO_THROW(app.framework.update());
                EXPECT_NO_THROW(app.framework.draw());
                ++iteration;
            }
        }

        void save_screenshot(ndbl::Nodable & app, const char* relative_path)
        {
            LOG_MESSAGE("Test", "Taking screenshot ...\n");
            ghc::filesystem::path path {relative_path};
            if( path.is_relative() )
            {
                path = ghc::filesystem::path(fw::system::get_executable_directory()) / "screenshots" / path;
            }
            if (!ghc::filesystem::exists(path.parent_path()))
            {
                create_directories(path.parent_path());
            }
            app.framework.save_screenshot(path.string().c_str());
        }
    };
}