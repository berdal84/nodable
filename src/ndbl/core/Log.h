#pragma once

#include "bdc/String.hpp"
#include "bdc/String_Builder.hpp"
#include "Format.h"
#include <chrono>
#include <ctime>
#include <deque>
#include <map>

#define NDBL_COLOR_DEFAULT     "\033[0m"
#define NDBL_COLOR_BLACK       "\033[30m"        /* Black */
#define NDBL_COLOR_RED         "\033[31m"        /* Red */
#define NDBL_COLOR_GREEN       "\033[32m"        /* Green */
#define NDBL_COLOR_YELLOW      "\033[33m"        /* Yellow */
#define NDBL_COLOR_BLUE        "\033[34m"        /* Blue */
#define NDBL_COLOR_MAGENTA     "\033[35m"        /* Magenta */
#define NDBL_COLOR_CYAN        "\033[36m"        /* Cyan */
#define NDBL_COLOR_WHITE       "\033[37m"        /* White */
#define NDBL_COLOR_BOLDBLACK   "\033[1m\033[30m" /* Bold Black */
#define NDBL_COLOR_BOLDRED     "\033[1m\033[31m" /* Bold Red */
#define NDBL_COLOR_BOLDGREEN   "\033[1m\033[32m" /* Bold Green */
#define NDBL_COLOR_BOLDYELLOW  "\033[1m\033[33m" /* Bold Yellow */
#define NDBL_COLOR_BOLDBLUE    "\033[1m\033[34m" /* Bold Blue */
#define NDBL_COLOR_BOLDMAGENTA "\033[1m\033[35m" /* Bold Magenta */
#define NDBL_COLOR_BOLDCYAN    "\033[1m\033[36m" /* Bold Cyan */
#define NDBL_COLOR_BOLDWHITE   "\033[1m\033[37m" /* Bold White */

#define NDBL_KO   NDBL_COLOR_BOLDRED "[KO]" NDBL_COLOR_DEFAULT // red   colored "[KO]" string.
#define NDBL_OK NDBL_COLOR_BOLDGREEN "[OK]" NDBL_COLOR_DEFAULT // green colored "[OK]" string.

#define NDBL_LOG(...) log( __VA_ARGS__ )

#ifdef NDBL_DEBUG
#   define NDBL_DEBUG_LOG(...)         NDBL_LOG( __VA_ARGS__ )
#   define NDBL_LOG_MESSAGE_MAX_COUNT  500000
#   define NDBL_LOG_VERBOSITY_DEFAULT  Verbosity_Diagnostic
#   define NDBL_LOG_MESSAGE_MEMORY_MAX 400*1000*1000
#else
#   define NDBL_DEBUG_LOG(...)         /* NDBL_DEBUG_LOG disabled when NDBL_DEBUG is false */
#   define NDBL_LOG_MESSAGE_MAX_COUNT  1000
#   define NDBL_LOG_VERBOSITY_DEFAULT  Verbosity_Message
#   define NDBL_LOG_MESSAGE_MEMORY_MAX 2*1000*1000
#endif

#define NDBL_ERROR   Verbosity_Error
#define NDBL_WARNING Verbosity_Warning
#define NDBL_MESSAGE Verbosity_Message
#define NDBL_DIAG    Verbosity_Diagnostic

namespace ndbl
{
    using namespace bdc;

    // Different verbosity levels a message can have
    typedef int Verbosity;
    enum Verbosity_: int
    {
        Verbosity_Error      = 0, // lowest level (always logged)
        Verbosity_Warning    = 1,
        Verbosity_Message    = 2,
        Verbosity_Diagnostic = 3, // highest level (rarely logged)

        Verbosity_COUNT,
    };

    struct VerbosityFilter
    {
        bool data[Verbosity_COUNT]; // TODO: we could use flags

        VerbosityFilter(bool default_value = false)
        {
            reset_all(default_value);
        }

        bool all_checked() const
        {
            for( int i = 0; i < Verbosity_COUNT; ++i )
                if ( !data[i] )
                    return false;
            return true;
        }

        void reset_all(bool default_value = false)
        {
            for( int i = 0; i < Verbosity_COUNT; ++i )
                data[i] = default_value;
        }

    };

    struct MessageData
    {
        using clock_t = std::chrono::time_point<std::chrono::system_clock>;

        const char*     category;   // short category name (ex: "Game", "App", etc.)
        Verbosity       verbosity;  // verbosity level
        String     text;       // message content
        clock_t         date;       // printed at date
    };

    struct LogState
    {
        Verbosity                        verbosity = NDBL_LOG_VERBOSITY_DEFAULT;
        std::map<u64_t, Verbosity>       verbosity_by_category_hash = {};
        std::deque<MessageData>          messages = {};
        VerbosityFilter                  verbosity_filter{true}; // true => checked by default
    };

    LogState&        get_log_state();
    void             set_log_verbosity(const String& category, Verbosity level); // Set verbosity level for a given category
    void             set_log_verbosity(Verbosity level); // override verbosity globally
    Verbosity        get_log_verbosity(const String& category);
    static Verbosity get_log_verbosity() { return get_log_state().verbosity; }
    void             flush(); // Ensure all messages have been printed out
    bool             show_log_message(const MessageData&, const VerbosityFilter& filter); // return true if messages needs to be displayed depending on filter and global verbosity

    template<typename...Args>
    void log(Verbosity verbosity, const char* category, const char* fmt, Args... args) // print a message like "[time|verbosity|category] message"
    {
        struct VerbosityInfo
        {
            const char* label;
            const char* color;
        };

        constexpr VerbosityInfo verbosity_info[Verbosity_COUNT] = {
            { "ERROR"     , NDBL_COLOR_RED     }, // Verbosity_Error
            { "WARNING"   , NDBL_COLOR_MAGENTA }, // Verbosity_Warning
            { "MESSAGE"   , NDBL_COLOR_DEFAULT }, // Verbosity_Message
            { "DIAGNOSTIC", NDBL_COLOR_DEFAULT }, // Verbosity_Diagnostic
        };

        MessageData message{};

        message.date            = std::chrono::system_clock::now();
        message.verbosity       = verbosity;
        message.category        = category;

        // message
        String_Builder sb{};
        string_builder_init(sb);
        string_builder_appendf(sb, "[%s|%s|%s] ", Format::tprint_time_point(message.date).c_str(), verbosity_info[verbosity].label, category);
        string_builder_appendf(sb, fmt, args...);

        message.text = string_builder_build_tstring(sb);

        // print if allowed
        if ( message.verbosity <= get_log_verbosity(category) )
        {
            // Print colored content
            printf("%s%s%s",
                verbosity_info[verbosity].color,
                message.text.c_str(),
                NDBL_COLOR_DEFAULT);

            // add to logs
            get_log_state().messages.emplace_front(message);
        }

        // Constraint the queue to have a limited size
        if ( get_log_state().messages.size() > NDBL_LOG_MESSAGE_MAX_COUNT )
        {
            get_log_state().messages.resize( NDBL_LOG_MESSAGE_MAX_COUNT / 2 );
        }
    }

    static_assert(
        NDBL_LOG_MESSAGE_MAX_COUNT * (sizeof(MessageData)) < NDBL_LOG_MESSAGE_MEMORY_MAX,
        "tools' log messages can go above limit");
}
