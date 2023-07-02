#pragma once

#include <deque>
#include <string>
#include <map>
#include <chrono>
#include <fw/core/string.h>

#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define MAGENTA "\033[35m"      /* Magenta */
#define CYAN    "\033[36m"      /* Cyan */
#define WHITE   "\033[37m"      /* White */
#define BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
#define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
#define BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
#define BOLDWHITE   "\033[1m\033[37m"      /* Bold White */

#define KO RED "[KO]" RESET      // red colored "[KO]" string.
#define OK GREEN "[OK]" RESET    // green colored "[OK]" string.

#define LOG_DISABLE_VERBOSE

#   define LOG_ERROR(...)   fw::log::push_message( fw::log::Verbosity_Error  , __VA_ARGS__ ); fw::log::flush();
#   define LOG_WARNING(...) fw::log::push_message( fw::log::Verbosity_Warning, __VA_ARGS__ );
#   define LOG_MESSAGE(...) fw::log::push_message( fw::log::Verbosity_Message, __VA_ARGS__ );
#   define LOG_FLUSH()      fw::log::flush();

#ifndef LOG_DISABLE_VERBOSE
#   define LOG_VERBOSE(...) fw::log::push_message( fw::log::Verbosity_Verbose, __VA_ARGS__ );
#else
#   define LOG_VERBOSE(...)
#endif

namespace fw {

	class log
    {
    public:

        // Different verbosity levels a message can have
        enum Verbosity: size_t
        {
            Verbosity_Error,          // highest level (always logged)
            Verbosity_Warning,
            Verbosity_Message,
            Verbosity_Verbose,
            Verbosity_COUNT,
            Verbosity_DEFAULT = Verbosity_Message
        };
        static const char* to_string(Verbosity _verbosity);

        struct Message
        {
            fw::string256 text{};
            fw::string32  category{};      // short category name (ex: "Game", "App", etc.)
            std::chrono::time_point<std::chrono::system_clock>
                          date = std::chrono::system_clock::now();
            Verbosity     verbosity=Verbosity_DEFAULT; // verbosity level
        };

	private:
        static std::deque<Message>  s_logs;      // message history
        static Verbosity            s_verbosity; // global verbosity level
        static std::map<std::string, Verbosity>& get_verbosity_by_category();

	public:
        static const std::deque<Message>& get_messages(); // Get message history
	    static void           set_verbosity(const std::string& _category, Verbosity _level) // Set verbosity level for a given category
        { get_verbosity_by_category().insert_or_assign(_category, _level ); }

	    inline static void    set_verbosity(Verbosity _level)                           // Set global verbosity level (for all categories)
        {
            s_verbosity = _level;
            get_verbosity_by_category().clear(); // ensure no overrides remains
        }

        static Verbosity        get_verbosity(const std::string& _category);            // Get verbosity level for a given category
        inline static Verbosity get_verbosity() { return s_verbosity; }                 // Get global verbosity level
        static void             flush();                                                // Ensure all messages have been printed out
        static void             push_message(Verbosity, const char* _category, const char* _format, ...); // Push a new message for a given category
    };
}