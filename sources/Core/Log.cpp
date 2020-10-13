#include "Log.h"

#include <cstdarg> // va_list, va_start, va_end
#include <cstdio>  // vfprintf
#include <iostream>

using namespace Nodable;

std::vector<Message> Log::Logs;

void Log::Push(LogType _type, short unsigned int _verbosity, const char* _format, ...)
{
    // Build log string
	char buffer[255];
    va_list arglist;
    va_start( arglist, _format );
    vsnprintf(buffer, sizeof(buffer), _format, arglist); // store into buffer
	va_end( arglist );

	// Print log
    if( _type == LogType::Error )
        std::cout << RED "ERR " RESET << buffer;
    else if( _type == LogType::Warning )
        std::cout << MAGENTA "WRN " RESET << buffer;
    else
        std::cout << "MSG " << buffer;

	// Store type and buffer in history
	Logs.push_back( {_type, _verbosity, buffer} );
}
