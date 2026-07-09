#pragma once

#include "ndbl/core/Nodable_Headless.h"
#include "ndbl/core/language/Nodlang.h"
#include "tools/core/File_System.h"
#include <exception>
#include <gtest/gtest.h>
#include <string>
#include <fstream>
#include <filesystem>

using namespace ndbl;
using namespace tools;

namespace testing
{
class Core : public Test
{
public:
    App_Headless_State state;

    void SetUp() override
    {
        nodable_init(&state);

        set_log_verbosity( Verbosity_Message );
        set_log_verbosity( "Parser", Verbosity_Diagnostic );
    }

    void TearDown() override
    {
        nodable_shutdown(&state);
    }

    std::string parse_and_serialize(const std::string &_source_code)
    {
        TOOLS_DEBUG_LOG(tools::Verbosity_Message, "core.h", "parse_and_serialize parsing \"%s\"\n", _source_code.c_str());

        // parse
        nodable_parse(&state, _source_code);

        // serialize
        std::string result;
        nodable_serialize(&state, result );
        TOOLS_DEBUG_LOG(tools::Verbosity_Message, "core.h", "parse_and_serialize serialize_node() output is: \"%s\"\n", result.c_str());

        return result;
    }

    // load a file relative to executable directory
    std::string load_file(const char* path)
    {
        tools::Path _path = tools::Path::get_executable_path().parent_path() / path;
        std::ifstream file_stream( _path.c_str() );
        if(!file_stream.is_open())
        {
            TOOLS_LOG(tools::Verbosity_Message, "core.h", "Can't open '%s'\n", _path.string().c_str() );
            ASSERT(false && "Unable to open file!" );
        }
        std::string program((std::istreambuf_iterator<char>(file_stream)), std::istreambuf_iterator<char>());
        return program;
    }
    
    void log_ribbon() const
    {
        TOOLS_LOG(tools::Verbosity_Message, "fixture::core", "%s\n\n", get_language()->_state.string().c_str());
    }
};
}
