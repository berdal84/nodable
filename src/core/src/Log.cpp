#include <nodable/core/Log.h>

#include <cstdarg> // va_list, va_start, va_end
#include <cstdio>  // vfprintf
#include <iostream>
#include <algorithm>

using namespace Nodable;

Log::Messages   Log::s_logs;
Log::Verbosity  Log::s_verbosity = Verbosity_Message;
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
        s_logs.push_front({_verbosity, _category, buffer} );

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
