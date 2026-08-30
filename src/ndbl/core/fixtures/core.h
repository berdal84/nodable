#pragma once

#include "ndbl/core/Nodable_Headless.h"
#include "ndbl/core/language/Nodlang.h"
#include "tools/core/File_System.h"
#include <exception>
#include <gtest/gtest.h>
#include "bdc/String.hpp"
#include <fstream>
#include <filesystem>

namespace testing
{
using namespace tools;
using namespace bdc;

class Core : public Test
{
public:
    App_Headless_State app;

    void SetUp() override
    {
        memory_manager_init();
        nodable_init(&app);

        set_log_verbosity( Verbosity_Message );
        set_log_verbosity( __FILE__, Verbosity_Diagnostic );
    }

    void TearDown() override
    {
        nodable_deinit(&app);
        memory_manager_shutdown();
    }

    String parse_and_serialize(const String &code)
    {
        TOOLS_DEBUG_LOG(Verbosity_Message, __FILE__, "parse_and_serialize parsing \"%s\"\n", code.c_str());

        nodable_parse(&app, code);
        String result = nodable_serialize(&app);

        TOOLS_DEBUG_LOG(Verbosity_Message, __FILE__, "parse_and_serialize serialize_node() output is: \"%s\"\n", result.c_str());

        return result;
    }

    // load a file relative to executable directory
    String load_file(const Path& path)
    {
        File_Read_Result result = file_read(path.c_str(), temp_allocator() );
        if(!result.ok)
        {
            TOOLS_LOG(Verbosity_Error, __FILE__, "%s\n", result.error.c_str() );
            ASSERT(false && "Unable to open file!" );
        }
        return result.content;
    }
    
    void log_ribbon() const
    {
        TOOLS_LOG(Verbosity_Message, "fixture::core", "%s\n\n", language().ribbon.to_string().c_str());
    }
};
}
