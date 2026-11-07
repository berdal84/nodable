#include "Log.h"
#include <iostream> // for std::flush and std::cout

namespace ndbl
{

static Log_State* g_log = nullptr;

void log_init()
{
    NDBL_ASSERT(g_log == nullptr, "log_init() has been called twice?");
    g_log = memory_new<Log_State>();
}

void log_shutdown()
{
    NDBL_ASSERT(g_log != nullptr, "log_shutdown() has been called twice?");
    for(auto& each : g_log->ring_buffer)
    {
        string_release(each.text);
    }
    memory_free(g_log);
    g_log = nullptr;
}

Log_State& log_manager()
{
    NDBL_ASSERT(g_log != nullptr, "Did you forgot to call log_init() ?");
    return *g_log;
}

void log_set_verbosity(const bdc::String& category, Verbosity level)
{
    auto& log = log_manager();
    hashmap_add( log.verbosity_by_category, category, level );
}

void log_set_verbosity(Verbosity level)
{
    auto& log = log_manager();
    log.verbosity = level;
    hashmap_clear( log.verbosity_by_category ); // ensure no overrides remains
}

Verbosity log_get_verbosity()
{
    return log_manager().verbosity;
}

Verbosity log_get_verbosity(const bdc::String& category)
{
    auto& log = log_manager();
    auto found = hashmap_find( log.verbosity_by_category, category );
    if ( found.ok )
    {
        return *found.data;
    }
    return log.verbosity;
}

Log_Entry& log_get_message_at(u32_t pos)
{
    auto& log = log_manager();
    u32_t ring_index = (log.ring_buffer_cursor + pos) % log.ring_buffer_size;
    return log.ring_buffer[ring_index];
}

void flush()
{
    std::cout << std::flush;
}

} // namespace ndbl