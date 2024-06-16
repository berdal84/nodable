#pragma once

#include "./string.h"
#include "format.h"
#include <chrono>
#include <ctime>
#include <deque>
#include <iostream>
#include <map>
#include <string>

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

#   define LOG_ERROR(...)                                                    \
    tools::log::push_message( tools::log::Verbosity_Error  , ##__VA_ARGS__ ); \
    tools::log::flush();
#   define LOG_WARNING(...) tools::log::push_message( tools::log::Verbosity_Warning, ##__VA_ARGS__ );
#   define LOG_MESSAGE(...) tools::log::push_message( tools::log::Verbosity_Message, ##__VA_ARGS__ );
#   define LOG_FLUSH() tools::log::flush();

#ifdef TOOLS_DEBUG
#   define LOG_VERBOSE(...) tools::log::push_message( tools::log::Verbosity_Verbose, ##__VA_ARGS__ );
#else
#   define LOG_VERBOSE(...)
#endif


namespace tools
{

	class log
    {
    public:
        struct Message;
        using MessageDeque = std::deque<Message>;

        // note: when size max is reached, half of the queue is cleared.
#ifdef TOOLS_DEBUG
        static constexpr size_t MESSAGE_MAX_COUNT = 500000; // n.b 1 Message == 368 bytes
#else
        static constexpr size_t MESSAGE_MAX_COUNT = 1000;
#endif

        // Different verbosity levels a message can have
        typedef int Verbosity;
        enum Verbosity_: int
        {
            Verbosity_Error, // lowest level (always logged)
            Verbosity_Warning,
            Verbosity_Message,
            Verbosity_Verbose, // highest level (rarely logged)
            Verbosity_COUNT,

#ifdef TOOLS_DEBUG
            Verbosity_DEFAULT = Verbosity_Verbose
#else
            Verbosity_DEFAULT = Verbosity_Message
#endif
        };

        static constexpr const char* to_string(log::Verbosity _verbosity)
        {
            switch (_verbosity)
            {
                case Verbosity_Error:   return  "ERR";
                case Verbosity_Warning: return  "WRN";
                case Verbosity_Message: return  "MSG";
                default:                return  "VRB";
            }
        }

        struct Message
        {
            using clock_t = std::chrono::time_point<std::chrono::system_clock>;

            string32  category{}; // short category name (ex: "Game", "App", etc.)
            Verbosity verbosity{Verbosity_DEFAULT}; // verbosity level
            string512 text{};
            clock_t   date{std::chrono::system_clock::now()};
        };

        static MessageDeque&    get_messages();
	    static void             set_verbosity(const std::string& _category, Verbosity _level); // Set verbosity level for a given category
	    static void             set_verbosity(Verbosity _level);                               // Set global verbosity level (for all categories)
        static Verbosity        get_verbosity(const std::string& _category); // Get verbosity level for a given category
        static Verbosity        get_verbosity();                             // Get global verbosity level
        static void             flush();                                     // Ensure all messages have been printed out

        template<typename...Args>
        static void push_message(
                Verbosity _verbosity,
                const char* _category,
                const char* _format,
                Args... args); // Push a new message for a given category

    private:
        static Verbosity            s_verbosity; // global verbosity level
        static std::map<std::string, Verbosity>& get_verbosity_by_category();
    };

    //
    // template declarations
    //

    template<typename...Args>
    void log::push_message(Verbosity _verbosity, const char* _category, const char* _format, Args... args)
    {
        // create a message like "[time|verbosity|category] message"
        Message message;
        message.verbosity = _verbosity;
        message.category = _category;
        // text prefix
        message.text.append_fmt(
                "[%s|%s|%s] ",
                format::time_point_to_string(message.date).c_str(),
                log::to_string(_verbosity),
                _category
                );
        // text body
        message.text.append_fmt(_format, args...);

        // print if allowed
        if ( message.verbosity <= s_verbosity )
        {
            // Select the appropriate color depending on the verbosity
            switch (_verbosity)
            {
                case log::Verbosity_Error:   std::cout << RED;      break;
                case log::Verbosity_Warning: std::cout << MAGENTA;  break;
                default:                     std::cout << RESET;  break;
            }

            // print the text and reset the color
            printf("%s" RESET, message.text.c_str());

            // add to logs
            get_messages().emplace_front(message);
        }

        // Constraint the queue to have a limited size
        if ( get_messages().size() > MESSAGE_MAX_COUNT )
        {
            get_messages().resize( MESSAGE_MAX_COUNT / 2 );
        }
    }

#if TOOLS_DEBUG
    static_assert( log::MESSAGE_MAX_COUNT * (sizeof(log::Message)) < 400*1000*1000, "Log messages in memory can potentially go above the limits (400MB in DEBUG)" );
#else
    static_assert( log::MESSAGE_MAX_COUNT * (sizeof(log::Message)) < 2*1000*1000, "Log messages in memory  can potentially go above the limits (2MB in Release)" );
#endif
}