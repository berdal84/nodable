#include "Log.h"
#include <iostream> // for std::flush and std::cout

namespace ndbl
{

static Log_State g_log = {};

Log_State& log_manager()
{
    return g_log;
}

void log_set_verbosity(const bdc::String& category, Verbosity level)
{
    hashmap_add( g_log.verbosity_by_category, category, level );
}

void log_set_verbosity(Verbosity level)
{
    g_log.verbosity = level;
    hashmap_clear( g_log.verbosity_by_category ); // ensure no overrides remains
}

Verbosity log_get_verbosity()
{
    return g_log.verbosity;
}

Verbosity log_get_verbosity(const bdc::String& category)
{
    auto found = hashmap_find( g_log.verbosity_by_category, category );
    if ( found.ok )
    {
        return *found.value;
    }
    return g_log.verbosity;
}

Log_Entry& log_get_message_at(u32_t pos)
{
    u32_t ring_index = (g_log.ring_buffer_cursor + pos) % g_log.ring_buffer_size;
    return g_log.ring_buffer[ring_index];
}

void flush()
{
    std::cout << std::flush;
}

} // namespace ndbl