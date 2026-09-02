#include "File_System.h"
#include <fstream>
#include "tools/core/Log.h"

#ifdef NDBL_DESKTOP
#include <whereami.h> // to locate executable directory
#endif

namespace tools
{
using namespace bdc;

const char* Path::c_str() const
{
    return m_path.c_str();
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
    Path result;
    result.m_path = m_path.filename();
    return result;
}

bool Path::empty() const
{
    return m_path.empty();
}

Path Path::parent_path() const
{
    Path result;
    result.m_path = m_path.parent_path();
    return result;
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
        path = memory_malloc_array<char>( length + 1, temp_allocator() );

        if ( wai_getExecutablePath(path, length, &dirname_length) )
        {
            path[length] = '\0';
            result = path;
            TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "system", "executable path: %s\n", result.c_str() );
            TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "system", "  dirname: %s\n", result.parent_path().c_str() );
            TOOLS_DEBUG_LOG(Verbosity_Diagnostic, "system", "  basename: %s\n", result.filename().c_str() );
        }
        else
        {
            TOOLS_LOG(Verbosity_Error, "system", "Unable to get executable path\n");
        }
    }
    else
    {
        TOOLS_LOG(Verbosity_Warning, "system", "Unable to get executable path!\n");
    }
    return result;
#else
    return "./fake-executable";
#endif
}

Path& Path::absolute()
{
    if ( this->is_absolute() )
        return *this;

    (*this) = Path::absolute(*this);
    return *this;
}

Path Path::absolute(const Path& _path)
{
    if ( _path.is_absolute() )
        return _path;
    // note: in __EMSCRIPTEN__, parent_path is "."
    Path result = Path::get_executable_path().parent_path() / _path;
    return result;
}

// TODO: this function is useless, we can use directly make_absolute
Path Path::get_asset_path(const String _str)
{
    return absolute(_str);
}

File_Read_Result file_read(const Path& path, bdc::Allocator* allocator)
{
    std::ifstream stream( path.c_str() );

    if (!stream.is_open())
    {
        return { .ok = false, .error = string_printf( temp_allocator(), "Unable to load \"%s\"", path.c_str()) };
    }

    Resizable_Array<i8_t> bytes;
    array_init(bytes, 1024, allocator); // let's use 1K minimum

    while( true )
    {
        char c = stream.get();
        
        if( stream.fail() )
        {
            if ( stream.eof() )
            {
                stream.clear();
                return { .ok = true, .content = bdc::array_view(bytes) }; 
            }
            return { .ok = false, .error = "Line exceeded buffer size!" };
        }

        // Since right now Resizable_Array<i8_t> reallocates linearly, we do a manual exponential resizesing
        if( bytes.capacity == bytes.size )
        {
            u32_t new_capacity = bytes.capacity * 2;
            if( new_capacity < bytes.capacity)
            {
                new_capacity = (u32_t)-1; // max
            }
            array_reserve_capacity_at_least( bytes, new_capacity );
        }
        array_append(bytes, c);
    }      
}

File_Write_Result file_write(const Path& path, const String& content)
{   
    std::ofstream out_fstream(path.c_str());
    
    if( out_fstream.is_open() )
    {
        return {.ok = false, .error = string_printf("Unable to open \"%s\"", path.c_str() )};
    }
    
    out_fstream.write(content.data, content.size);
    out_fstream.close();

    return { .ok = true };
}

} // namespace tools