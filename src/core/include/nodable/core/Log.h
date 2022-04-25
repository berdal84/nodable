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

#define KO RED "[KO]" RESET
#define OK GREEN "[OK]" RESET

#define LOG_ENABLE true

#if LOG_ENABLE
#   define LOG_ERROR(...)   ndbl::Log::push_message( ndbl::Log::Verbosity_Error  , __VA_ARGS__ ); ndbl::Log::flush();
#   define LOG_WARNING(...) ndbl::Log::push_message( ndbl::Log::Verbosity_Warning, __VA_ARGS__ );
#   define LOG_MESSAGE(...) ndbl::Log::push_message( ndbl::Log::Verbosity_Message, __VA_ARGS__ );
#   define LOG_VERBOSE(...) ndbl::Log::push_message( ndbl::Log::Verbosity_Verbose, __VA_ARGS__ );
#   define LOG_FLUSH()      ndbl::Log::flush();
#else
#   define LOG_ERROR(...)
#   define LOG_WARNING(...)
#   define LOG_MESSAGE(...)
#   define LOG_VERBOSE(...)
#   define LOG_FLUSH()
#endif

namespace ndbl {

	class Log
    {
    public:

        enum Verbosity: size_t
        {
            Verbosity_Error,
            Verbosity_Warning,
            Verbosity_Message,
            Verbosity_Verbose,
            Verbosity_COUNT
        };

        struct Message
        {
            using time_point_t = std::chrono::time_point<std::chrono::system_clock>;
            time_point_t time_point;
            Verbosity    verbosity;
            std::string  category;
            std::string  text;
            std::string  to_string()const;
            std::string  to_full_string()const;
        };

        using Messages = std::deque<Message>;
	private:
        static Messages s_logs;
        static std::map<std::string, Verbosity> s_verbosity_by_category;
        static Verbosity                        s_verbosity;

	public:
        static const Messages& get_messages();
        static const Message& get_last_message();
	    static void           set_verbosity(const std::string& _category, Verbosity _verbosityLevel);
	    static void           set_verbosity(Verbosity);
        static Verbosity      get_verbosity(const std::string& _category);
        static Verbosity      get_verbosity();
		static void           push_message(Verbosity _verbosity, const char* _category, const char* _format, ...);
		static void           flush();

        static std::string to_string(Verbosity _verbosity);
    };
}