#include <gtest/gtest.h>

#include "ndbl/gui/Nodable.h"
#include "ndbl/gui/Nodable_View.h"

#ifdef NDBL_GUI_TEST_HUMAN_SPEED
#include <thread>
#define SLEEP_FOR_HUMAN( delay_in_ms ) std::this_thread::sleep_for( std::chrono::milliseconds( delay_in_ms ) );
#else
#define SLEEP_FOR_HUMAN( delay_in_ms )
#endif

namespace testing
{
    using namespace ndbl;
    using namespace bdc;

    class Nodable_Gui_Test: public Test
    {
    public:
        Nodable_Gui_Test() {};

        void SetUp() override
        {
            app_init();
        }

        void TearDown() override
        {
            app_shutdown();
        }
        
        static void loop_for_n_frame(size_t frame_count, double sleep_in_sec = 1.0)
        {
            for(size_t i = 0; i < frame_count; ++i)
            {
                EXPECT_NO_THROW(app_do_frame());
            }
            SLEEP_FOR_HUMAN((long)(1000.0 * sleep_in_sec));
        }

        /**
         * run some loops for a given duration
         */
        static void loop_for_n_sec(double total_duration_in_sec)
        {
            auto   start = std::chrono::system_clock::now();
            auto   end   = std::chrono::system_clock::now();
            size_t frame = 0;
            while((std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() < (long long)(total_duration_in_sec * 1000.f)))
            {
                end = std::chrono::system_clock::now();
                float elapsed_in_sec = float(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()) / 1000.0f;
                NDBL_LOG(Verbosity_Message, "Test", "Loop frame %llu (time: %0.1f/%0.1f sec)\n", frame, elapsed_in_sec, total_duration_in_sec);
                EXPECT_NO_THROW(app_do_frame());
                ++frame;
            }
        }
    };
}