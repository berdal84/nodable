#pragma once

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

#ifdef _DEBUG
	#define LOG_DEBUG(...) Nodable::internal::LogMessage( ""  , __VA_ARGS__ ) 
#else
	#define LOG_DEBUG(...)
#endif

#define LOG_MESSAGE(...) Nodable::internal::LogMessage( "", __VA_ARGS__ ) 
#define LOG_WARNING(...) Nodable::internal::LogMessage( MAGENTA "WRN " RESET, __VA_ARGS__ )
#define LOG_ERROR(...)   Nodable::internal::LogMessage( RED "ERR " RESET, __VA_ARGS__ ) 

namespace Nodable{	
	namespace internal
	{
		void LogMessage(const char* _prefix, const char* _format, ...);
	}
}