#include "log.h"

#include <iostream>

using namespace tools;

log::Verbosity log::s_verbosity = Verbosity_DEFAULT;

std::deque<log::Message>& log::get_logs()
{
    static std::deque<log::Message> logs;
    return logs;
}

log::Message& log::create_message()
{
    return get_logs().emplace_front(); // Store a new message in the front of the queue
}

std::map<std::string, log::Verbosity>& log::get_verbosity_by_category()
{
    // use singleton pattern instead of static member to avoid static code issues
    static std::map<std::string, log::Verbosity> verbosity_by_category;
    return verbosity_by_category;
}

log::Verbosity log::get_verbosity(const std::string& _category)
{
    std::map<std::string, log::Verbosity>& verbosity_by_category = get_verbosity_by_category();
    const auto& pair = verbosity_by_category.find(_category);
    if (pair != verbosity_by_category.end() )
    {
        return pair->second;
    }
    return s_verbosity;
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
