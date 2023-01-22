#pragma once

#include <deque>
#include <string>
#include <map>
#include <chrono>

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

#define LOG_ENABLE true

#if LOG_ENABLE
#   define LOG_ERROR(...)   fw::Log::push_message( fw::Log::Verbosity_Error  , __VA_ARGS__ ); fw::Log::flush();
#   define LOG_WARNING(...) fw::Log::push_message( fw::Log::Verbosity_Warning, __VA_ARGS__ );
#   define LOG_MESSAGE(...) fw::Log::push_message( fw::Log::Verbosity_Message, __VA_ARGS__ );
#   define LOG_VERBOSE(...) fw::Log::push_message( fw::Log::Verbosity_Verbose, __VA_ARGS__ );
#   define LOG_FLUSH()      fw::Log::flush();
#else
#   define LOG_ERROR(...)
#   define LOG_WARNING(...)
#   define LOG_MESSAGE(...)
#   define LOG_VERBOSE(...)
#   define LOG_FLUSH()
#endif

namespace fw {

	class Log
    {
    public:

        // Different verbosity levels a log can have
        enum Verbosity: size_t
        {
            Verbosity_Error,          // highest level (always logged)
            Verbosity_Warning,
            Verbosity_Message,
            Verbosity_Verbose,        // lowest level
            Verbosity_COUNT,
            Verbosity_DEFAULT = Verbosity_Message
        };
        static std::string to_string(Verbosity _verbosity);

        struct Message
        {
            using time_point_t = std::chrono::time_point<std::chrono::system_clock>;
            time_point_t date;                 // message date
            Verbosity    verbosity;            // verbosity level
            std::string  category;             // category (ex: "Game", "App", etc.)
            std::string  text;                 // message content text
            std::string  to_string()const;     // get a pretty string of this message (ex: "[MSG|Game] Starting ...")
            std::string  to_full_string()const;// get a pretty string of this message (ex: "[<date>|MSG|Game] Starting ...")
        };

        using Messages = std::deque<Message>;
	private:
        static Messages  s_logs;      // message history
        static Verbosity s_verbosity; // current global verbosity level
        static std::map<std::string, Verbosity>& get_verbosity_by_category();

	public:
        static const Messages& get_messages();
        static const Message& get_last_message();
	    static void           set_verbosity(const std::string& _category, Verbosity);
	    static void           set_verbosity(Verbosity);
        static Verbosity      get_verbosity(const std::string& _category);
        static Verbosity      get_verbosity();
		static void           push_message(Verbosity _verbosity, const char* _category, const char* _format, ...);
		static void           flush();
    };
}