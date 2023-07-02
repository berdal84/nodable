#include "fw/core/log.h"

#include <cstdarg> // va_list, va_start, va_end
#include <cstdio>  // vfprintf
#include <iostream>
#include <algorithm>
#include <ctime>
#include <cstring>

using namespace fw;

std::deque<log::Message> log::s_logs;
log::Verbosity           log::s_verbosity = Verbosity_DEFAULT;

static fw::string32 time_point_to_string(const std::chrono::system_clock::time_point &time_point)
{
    std::time_t time = std::chrono::system_clock::to_time_t(time_point);
    // The 25th char contains '\n'
#ifdef WIN32
    char str[26];
    ctime_s(str, sizeof str, &time);
    return {str, 25}
#else
    return {ctime(&time), 25};
#endif
}

std::map<std::string, log::Verbosity>& log::get_verbosity_by_category()
{
    // use singleton pattern instead of static member to avoid static code issues
    static std::map<std::string, log::Verbosity> verbosity_by_category;
    return verbosity_by_category;
}

void log::set_verbosity(const std::string& _category, Verbosity _level)
{
   get_verbosity_by_category().insert_or_assign(_category, _level );
}

void log::set_verbosity(Verbosity _level)
{
    s_verbosity = _level;
    get_verbosity_by_category().clear(); // ensure no overrides remains
}

log::Verbosity log::get_verbosity()
{
    return s_verbosity;
}

log::Verbosity log::get_verbosity(const std::string& _category)
{
    std::map<std::string, log::Verbosity>& verbosity_by_category = get_verbosity_by_category();
    auto pair = verbosity_by_category.find(_category);
    if (pair != verbosity_by_category.end() ) return pair->second;
    return s_verbosity;
}

void log::push_message(Verbosity _verbosity, const char* _category, const char* _format, ...)
{
	// Print log only if verbosity level allows it

	if (_verbosity <= get_verbosity(_category) )
    {
        Message message{};
        message.verbosity = _verbosity;
        message.category  = _category;

        message.text.push_back('[');
        message.text.append( time_point_to_string(message.date) );
        message.text.push_back('|');
        message.text.append(log::to_string(_verbosity));
        message.text.push_back('|');
        message.text.append( _category );
        message.text.push_back(']');
        message.text.push_back(' ');

        // Fill a buffer with the formatted message
        va_list arglist;
        va_start( arglist, _format );
        message.text.append_fmt(_format, arglist);
        va_end( arglist );

        // Select the appropriate color depending on the verbosity
        switch (_verbosity)
        {
            case log::Verbosity_Error:   std::cout << RED;      break;
            case log::Verbosity_Warning: std::cout << MAGENTA;  break;
            default: /* uses default terminal color */;
        }

        // print the text
        printf("%s", message.text.c_str());

        // Store the message in the front of the queue
        s_logs.push_front(message);

        // Constraint the queue to have a limited size
        constexpr size_t max_count = 5000; // a Message is 512 bytes
        constexpr size_t min_count = 4000; //
        if (s_logs.size() > max_count ) s_logs.resize(min_count);
    }

}

const char* log::to_string(log::Verbosity _verbosity)
{
    switch (_verbosity)
    {
        case Verbosity_Error:   return  "ERR";
        case Verbosity_Warning: return  "WRN";
        case Verbosity_Message: return  "MSG";
        default:                return  "VRB";
    }
}

void log::flush()
{
    std::cout << std::flush;
}

const std::deque<log::Message>& log::get_messages()
{
    return s_logs;
}
