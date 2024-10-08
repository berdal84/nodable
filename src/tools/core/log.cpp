#include "log.h"

using namespace tools;

log::Verbosity log::s_verbosity = Verbosity_DEFAULT;


log::MessageDeque& log::get_messages()
{
    static std::deque<log::Message> logs;
    return logs;
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

log::Verbosity log::get_verbosity()
{
    return s_verbosity;
}

void log::flush()
{
    std::cout << std::flush;
}