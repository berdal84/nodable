#include "fw/core/Log.h"

#include <cstdarg> // va_list, va_start, va_end
#include <cstdio>  // vfprintf
#include <iostream>
#include <algorithm>
#include <ctime>
#include <cstring>

using namespace fw;

Log::Messages   Log::s_logs;
Log::Verbosity  Log::s_verbosity = Verbosity_DEFAULT;

std::map<std::string, Log::Verbosity>& Log::get_verbosity_by_category()
{
    // use singleton pattern to avoid static code issues
    static std::map<std::string, Log::Verbosity> verbosity_by_category;

    return verbosity_by_category;
}

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
   get_verbosity_by_category().insert_or_assign(_category, _level );
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
    auto verbosity_by_category = get_verbosity_by_category();
    auto pair = verbosity_by_category.find(_category);
    if (pair != verbosity_by_category.end() )
    {
        return pair->second;
    }
    return s_verbosity;
}

void Log::push_message(Verbosity _verbosity, const char* _category, const char* _format, ...)
{
	// Print log only if verbosity level allows it

	if (_verbosity <= get_verbosity(_category) )
    {
        // Build log string
        char buffer[255];
        va_list arglist;
        va_start( arglist, _format );
        vsnprintf(buffer, sizeof(buffer), _format, arglist); // store into buffer
        va_end( arglist );

        // select a color
        switch (_verbosity)
        {
            case Log::Verbosity_Error:   std::cout << RED;      break;
            case Log::Verbosity_Warning: std::cout << MAGENTA;  break;
            default:;
        }

        // print the text
        std::cout << "[" << to_string(_verbosity) << "|" << _category << "] ";
        std::cout << RESET;
        std::cout << buffer;

        // Store type and buffer in history
        s_logs.push_front({std::chrono::system_clock::now(), _verbosity, _category, buffer} );

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

std::string Log::to_string(Log::Verbosity _verbosity)
{
    switch (_verbosity)
    {
        case Verbosity_Error:   return  "ERR";
        case Verbosity_Warning: return  "WRN";
        case Verbosity_Message: return  "MSG";
        default:                return  "VRB";
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

static std::string time_point_to_string(const std::chrono::system_clock::time_point &time_point)
{
    std::time_t time = std::chrono::system_clock::to_time_t(time_point);
    char result[26];
    ctime_s(result,sizeof result,&time);
    result[strlen(result) - 1] = '\0'; // prevent new line
    return result;
}

std::string Log::Message::to_full_string()const
{
    std::string result;
    result.reserve(50);

    result.push_back('[');
    result.append( time_point_to_string(date) );
    result.push_back('|');
    result.append( Log::to_string(verbosity) );
    result.push_back('|');
    result.append( category );
    result.push_back(']');
    result.push_back(' ');
    result.append( text );

    return result;
}

std::string Log::Message::to_string()const
{
    std::string result;
    result.reserve(50);

    result.push_back('[');
    result.append( Log::to_string(verbosity) );
    result.push_back('|');
    result.append( category );
    result.push_back(']');
    result.push_back(' ');
    result.append( text );

    return result;
}
