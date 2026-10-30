#pragma once

#include "bdc/String.hpp"
#include "bdc/String_Builder.hpp"
#include "bdc/Array.hpp"
#include "bdc/Hash_Map.hpp"
#include "Format.h"
#include <chrono>
#include <ctime>
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
#   define NDBL_LOG_VERBOSITY_DEFAULT  Verbosity_Diagnostic
#else
#   define NDBL_DEBUG_LOG(...)         /* NDBL_DEBUG_LOG disabled when NDBL_DEBUG is false */
#   define NDBL_LOG_VERBOSITY_DEFAULT  Verbosity_Message
#endif

#define NDBL_ERROR   Verbosity_Error
#define NDBL_WARNING Verbosity_Warning
#define NDBL_MESSAGE Verbosity_Message
#define NDBL_DIAG    Verbosity_Diagnostic

namespace ndbl
{
    using namespace bdc;

    // Different verbosity levels a message can have
    enum Verbosity: i8_t
    {
        Verbosity_Diagnostic, // rarely logged
        Verbosity_Message   , // ..
        Verbosity_Warning   , // ..
        Verbosity_Error     , // always logged

        Verbosity_COUNT,
    };

    // Verbosity_Flags to make binary masks (to toggle visibility on a per-level basis)
    typedef u8_t Verbosity_Flags;

    // helper to convert 1 or more verbosity levels to verbosity flags.
    template<typename... T>
    constexpr Verbosity_Flags verbosity_flags(T... values)
    {
        return ((1u << values) | ...);
    }

    enum Verbosity_Flags_: u8_t
    {
        Verbosity_Flags_Error      = verbosity_flags(Verbosity_Error),
        Verbosity_Flags_Warning    = verbosity_flags(Verbosity_Warning), 
        Verbosity_Flags_Message    = verbosity_flags(Verbosity_Message), 
        Verbosity_Flags_Diagnostic = verbosity_flags(Verbosity_Diagnostic), 

        Verbosity_Flags_All        = verbosity_flags(Verbosity_Diagnostic,
                                                     Verbosity_Message,
                                                     Verbosity_Warning,
                                                     Verbosity_Error)
    };

    struct Log_Entry
    {
        using clock_t = std::chrono::time_point<std::chrono::system_clock>;

        const char* category;   // short category name (ex: "Game", "App", etc.)
        Verbosity   verbosity;  // verbosity level
        String      text;       // message content
        clock_t     date;       // printed at date
    };

    struct Log_State
    {
        Verbosity                        verbosity              = NDBL_LOG_VERBOSITY_DEFAULT;
        Hash_Map<bdc::String, Verbosity> verbosity_by_category  = {};
        static constexpr u32_t           ring_buffer_size       = 1024*16; // max line count that can be hold in memory
        u32_t                            ring_buffer_cursor     = 0; // index of the latest message
        Log_Entry                        ring_buffer[ring_buffer_size]= {0};
    };

    Log_State&       log_manager();
    void             log_set_verbosity(Verbosity); // reset verbosity globally (clear any category override)
    void             log_set_verbosity(const String& category, Verbosity); // override for a given category
    Verbosity        log_get_verbosity();
    Verbosity        log_get_verbosity(const String& category);
    Log_Entry&       log_get_message_at(u32_t pos);
    void             flush(); // Ensure all messages have been printed out

    template<typename...Args>
    void log(Verbosity verbosity, const char* category, const char* fmt, Args... args) // print a message like "[time|verbosity|category] message"
    {
        if ( verbosity < log_get_verbosity(category) )
        {
            return;
        }

        Log_State& state = log_manager();

        struct VerbosityInfo
        {
            const char* label;
            const char* color;
        };

        constexpr VerbosityInfo verbosity_info[Verbosity_COUNT] = {
            { "DIAGNOSTIC", NDBL_COLOR_DEFAULT }, // Verbosity_Diagnostic
            { "MESSAGE"   , NDBL_COLOR_DEFAULT }, // Verbosity_Message
            { "WARNING"   , NDBL_COLOR_MAGENTA }, // Verbosity_Warning
            { "ERROR"     , NDBL_COLOR_RED     }, // Verbosity_Error
        };

        Log_Entry msg = {};

        msg.date      = std::chrono::system_clock::now();
        msg.verbosity = verbosity;
        msg.category  = category;

        // message
        String_Builder sb{};
        string_builder_init(sb);
        string_builder_appendf(sb, "[%s|%s|%s] ", Format::tprint_time_point(msg.date).c_str(), verbosity_info[verbosity].label, category);
        string_builder_appendf(sb, fmt, args...);

        msg.text = string_builder_build_string(sb);

        // print to stdout
        printf("%s%s%s",
            verbosity_info[verbosity].color,
            msg.text.c_str(),
            NDBL_COLOR_DEFAULT);

        // store in message ring and advance cursor
        string_release(state.ring_buffer[state.ring_buffer_cursor].text); // release old message
        state.ring_buffer[state.ring_buffer_cursor] = msg;
        state.ring_buffer_cursor = (state.ring_buffer_cursor+1) % state.ring_buffer_size;
    }
}
