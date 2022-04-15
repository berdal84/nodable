#include <nodable/core/Log.h>

#include <cstdarg> // va_list, va_start, va_end
#include <cstdio>  // vfprintf
#include <iostream>
#include <algorithm>

using namespace Nodable;

Log::Messages   Log::s_logs;
Log::Verbosity  Log::s_verbosity = Verbosity::Message;
std::map<std::string, Log::Verbosity> Log::s_verbosity_by_category;

const Log::Message& Log::get_last_message()
{
#if LOG_ENABLE
    return  Log::s_logs.front();
#else
    return nullptr;
#endif
}

void Log::set_verbosity(const std::string& _category, Verbosity _level)
{
   s_verbosity_by_category.insert_or_assign(_category, _level );
}

void Log::set_verbosity(Verbosity _level)
{
    s_verbosity = _level;
}
Log::Verbosity Log::get_verbosity()
{
    return s_verbosity;
}

Log::Verbosity Log::get_verbosity(const std::string& _category)
{
    auto pair = s_verbosity_by_category.find(_category);
    if (pair != s_verbosity_by_category.end() )
    {
        return pair->second;
    }
    return s_verbosity;
}

void Log::push_message(Verbosity _verbosityLevel, const char* _category, const char* _format, ...)
{
	// Print log only if verbosity level allows it

	if (_verbosityLevel <= get_verbosity(_category) )
    {
        // Build log string
        char buffer[255];
        va_list arglist;
        va_start( arglist, _format );
        vsnprintf(buffer, sizeof(buffer), _format, arglist); // store into buffer
        va_end( arglist );

        // Print the verbosity:
        switch (_verbosityLevel)
        {
            case Log::Verbosity::Error:   std::cout << RED "[ERR|";      break;
            case Log::Verbosity::Warning: std::cout << MAGENTA "[WRN|";  break;
            case Log::Verbosity::Message: std::cout << "[MSG|";          break;
            default:                      std::cout << "[VRB|";
        }

        // the text
        std::cout << _category << "] " RESET << buffer;

        // Store type and buffer in history
        s_logs.push_front({_category, _verbosityLevel, buffer} );

        // Constraint the queue to be size() < 500
        // Erase by chunk of 250
        constexpr size_t chunk_size = 250;
        if (s_logs.size() > 2*chunk_size )
        {
            while( s_logs.size() > chunk_size )
            {
                s_logs.pop_back();
            }
        }
    }

}

void Log::flush()
{
    std::cout << std::flush;
}

const Log::Messages& Log::get_messages()
{
    return s_logs;
}
