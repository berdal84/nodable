#include "File_System.h"
#include "tools/core/Log.h"

#ifdef NDBL_DESKTOP
#include <whereami.h> // to locate executable directory
#endif

using namespace tools;

const std::filesystem::path::value_type* Path::c_str() const
{
    return m_path.c_str();
}

std::string Path::string() const
{
    return m_path.string();
}

bool Path::is_absolute() const
{
    return m_path.is_absolute();
}

bool Path::is_relative() const
{
    return m_path.is_relative();
}

Path Path::filename() const
{
    return m_path.filename();
}

bool Path::empty() const
{
    return m_path.empty();
}

Path Path::parent_path() const
{
    return m_path.parent_path();
}

bool Path::create_directories(const Path& path)
{
    return std::filesystem::create_directories(path.m_path);
}

Path Path::get_executable_path()
{
#ifdef NDBL_DESKTOP
    char* path = nullptr;
    int length, dirname_length;
    length = wai_getExecutablePath(nullptr, 0, &dirname_length);
    Path result;
    if (length > 0)
    {
        path = new char[length + 1];

        if ( wai_getExecutablePath(path, length, &dirname_length) )
        {
            path[length] = '\0';
            result = path;
            TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "tools::system", "executable path: %s\n", result.c_str() );
            TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "tools::system", "  dirname: %s\n", result.parent_path().c_str() );
            TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "tools::system", "  basename: %s\n", result.filename().c_str() );
        }
        else
        {
            TOOLS_LOG(tools::Verbosity_Error, "tools::system", "Unable to get executable path\n");
        }
        delete[] path;
    }
    else
    {
        TOOLS_LOG(tools::Verbosity_Warning, "tools::system", "Unable to get executable path!\n");
    }
    return result;
#else
    return "./fake-executable";
#endif
}

tools::Path& Path::absolute()
{
    if ( this->is_absolute() )
        return *this;

    (*this) = Path::absolute(*this);
    return *this;
}

tools::Path Path::absolute(const tools::Path& _path)
{
    if ( _path.is_absolute() )
        return _path;
    // note: in __EMSCRIPTEN__, parent_path is "."
    Path result = Path::get_executable_path().parent_path() / _path;
    return result;
}

// TODO: this function is useless, we can use directly make_absolute
tools::Path Path::get_asset_path(const char* _str)
{
    Path result = absolute(_str);
    return result;
}