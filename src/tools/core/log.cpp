#include "log.h"

#include <iostream>

using namespace tools;

std::deque<log::Message> log::s_logs;
log::Verbosity           log::s_verbosity = Verbosity_DEFAULT;

std::map<std::string, log::Verbosity>& log::get_verbosity_by_category()
{
    // use singleton pattern instead of static member to avoid static code issues
    static std::map<std::string, log::Verbosity> verbosity_by_category;
    return verbosity_by_category;
}

log::Verbosity log::get_verbosity(const std::string& _category)
{
    std::map<std::string, log::Verbosity>& verbosity_by_category = get_verbosity_by_category();
    auto pair = verbosity_by_category.find(_category);
    if (pair != verbosity_by_category.end() ) return pair->second;
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

const std::deque<log::Message>& log::get_messages()
{
    return s_logs;
}
