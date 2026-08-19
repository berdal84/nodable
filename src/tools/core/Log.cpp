#include "Log.h"
#include "bdc/String_Hash.hpp"
#include <iostream>

using namespace tools;

LogState& tools::get_log_state()
{
    static LogState state{};
    return state;
}

bool tools::show_log_message(const MessageData& message, const VerbosityFilter& filter)
{
    if ( message.verbosity <= get_log_verbosity( message.category ) )
        return filter.data[message.verbosity];
    return false;
}

void tools::set_log_verbosity(const bdc::String& category, Verbosity level)
{
    get_log_state().verbosity_by_category_hash.insert_or_assign( bdc::string_hash(category).hash , level );
}

void tools::set_log_verbosity(Verbosity level)
{
    get_log_state().verbosity = level;
    get_log_state().verbosity_by_category_hash.clear(); // ensure no overrides remains
}

Verbosity tools::get_log_verbosity(const bdc::String& category)
{
    const auto& pair = get_log_state().verbosity_by_category_hash.find( bdc::string_hash(category).hash );
    if (pair != get_log_state().verbosity_by_category_hash.end() )
    {
        return pair->second;
    }
    return get_log_state().verbosity;
}

void tools::flush()
{
    std::cout << std::flush;
}