#pragma once

#include <deque>
#include <string>
#include <map>

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
#   define LOG_ERROR(...)   Nodable::Log::push_message( Nodable::Log::Verbosity::Error  , __VA_ARGS__ ); Nodable::Log::flush();
#   define LOG_WARNING(...) Nodable::Log::push_message( Nodable::Log::Verbosity::Warning, __VA_ARGS__ );
#   define LOG_MESSAGE(...) Nodable::Log::push_message( Nodable::Log::Verbosity::Message, __VA_ARGS__ );
#   define LOG_VERBOSE(...) Nodable::Log::push_message( Nodable::Log::Verbosity::Verbose, __VA_ARGS__ );
#   define LOG_FLUSH()      Nodable::Log::flush();
#else
#   define LOG_ERROR(...)
#   define LOG_WARNING(...)
#   define LOG_MESSAGE(...)
#   define LOG_VERBOSE(...)
#   define LOG_FLUSH()
#endif

namespace Nodable {

	class Log
    {
    public:

        enum class Verbosity: int
        {
            Error,
            Warning,
            Message,
            Verbose
        };

        struct Message
        {
            std::string category;
            Verbosity   verbosity;
            std::string text;
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
		static void           push_message(Verbosity _verbosityLevel, const char* _category, const char* _format, ...);
		static void           flush();
    };
}