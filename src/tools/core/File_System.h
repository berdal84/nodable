#pragma once
#include <filesystem>
#include "bdc/String.hpp"

namespace tools
{
    // Wraps a std::filesystem object as a multi-platform interface (bdc::String under linux is super permissive, but it does not compiles on MSVC..)
    class Path
    {
    public:

        Path()
        : m_path()
        {}
        
        Path(const char* str)
        : m_path(str)
        {}

        Path(const bdc::String& str)
        : m_path(str.c_str())
        {}

        Path(const std::filesystem::path& str)
        : m_path(str)
        {}

        const char*  c_str()const;
        bool         is_absolute() const;
        bool         is_relative() const;
        Path         filename() const;
        bool         empty() const;
        Path         parent_path() const;
        Path&        absolute();

        static bool exists(const Path& path)
        { return std::filesystem::exists(path.m_path); };

        Path& operator=(const char* other)
        { m_path = other; return *this; }

        Path& operator/(const Path& other)
        { m_path /= other.m_path; return *this; }

        bool operator==(const Path& other)
        { return m_path == other.m_path; }

        static bool  create_directories(const Path&);
        static Path  get_executable_path();            // Get the executable directory absolute path
        static Path  absolute(const Path &_path);
        static Path  get_asset_path(const bdc::String _relative_path); // return a valid path (absolute or relative depending platform)

    private:
        std::filesystem::path m_path;
    };
}