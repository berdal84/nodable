#include "Log.h"

#include <cstdarg> // va_list, va_start, va_end
#include <cstdio>  // vfprintf
#include <iostream>

using namespace Nodable;

std::vector<Message> Log::Logs;

void Log::Push(LogType _type, const char* _format, ...)
{

	/* 1 - logs prefix */
	if( _type == LogType::Error )
		std::cout << RED "ERR " RESET;

	else if( _type == LogType::Warning )
		std::cout << MAGENTA "WRN " RESET;

	else
		std::cout << "MSG ";

	char buffer[255];

	/* 2 - logs text */
    va_list arglist;
    va_start( arglist, _format );
    vsnprintf(buffer, sizeof(buffer), _format, arglist);
    printf(_format, arglist);
	va_end( arglist );

	// Store type and buffer in history
	Logs.push_back( {_type, buffer} );
}
