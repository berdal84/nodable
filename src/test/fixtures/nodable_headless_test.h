#pragma once

#include <exception>
#include <gtest/gtest.h>
#include <fstream>
#include <filesystem>
#include "bdc/String.hpp"
#include "ndbl/core/File_System.h"
#include "ndbl/core/Nodable_Headless.h"
#include "ndbl/core/language/Nodlang.h"
#include "ndbl/core/reflection/index.h"
#include "ndbl/core/Log.h"

namespace testing
{
using namespace ndbl;
using namespace bdc;

class Nodable_Headless_Test : public Test
{
public:
    App_Headless_State app;

    Nodable_Headless_Test() {}
    
    void SetUp() override
    {
        nodable_init(&app);

        set_log_verbosity( Verbosity_Message );
        set_log_verbosity( __FILE__, Verbosity_Diagnostic );
    }

    void TearDown() override
    {
        nodable_deinit(&app);
    }

    String parse_and_serialize(const String &code)
    {
        NDBL_DEBUG_LOG(Verbosity_Message, __FILE_NAME__, "parse_and_serialize parsing \"%s\"\n", code.c_str());

        nodable_parse(&app, code);
        String result = nodable_serialize(&app);

        NDBL_DEBUG_LOG(Verbosity_Message, __FILE_NAME__, "parse_and_serialize serialize_node() output is: \"%s\"\n", result.c_str());

        return result;
    }

    // load a file relative to executable directory
    String load_file(const Path& path)
    {
        push_allocator(temp_allocator);
        File_Read_Result result = file_read(path.c_str());
        pop_allocator();
        if(!result.ok)
        {
            NDBL_LOG(Verbosity_Error, __FILE_NAME__, "%s\n", result.error.c_str() );
            VERIFY(false, "Unable to open file!" );
        }
        return result.content;
    }
    
    void log_ribbon() const
    {
        NDBL_LOG(Verbosity_Message, "fixture::core", "%s\n\n", language().ribbon.to_string().c_str());
    }
};
}
