#include "Log.h"

#include <cstdarg> // va_list, va_start, va_end
#include <cstdio>  // vfprintf
#include <iostream>
#include <algorithm>

using namespace Nodable;

std::vector<Message> Log::Logs;
short unsigned int Log::VerbosityLevel = Log::DefaultVerbosityLevel;

const Message* Log::GetLastMessage()
{
    auto found = std::find_if( Logs.rbegin(), Logs.rend(), [](auto item)-> bool {
        return item.verbosity <= Log::VerbosityLevel;
    });

    if (found != Logs.rend() )
    {
        return &(*found);
    }
    else
    {
        return nullptr;
    }

}

void Log::SetVerbosityLevel(short unsigned int _verbosityLevel)
{
    Log::VerbosityLevel = _verbosityLevel;
}

void Log::Push(LogType _type, short unsigned int _verbosityLevel, const char* _format, ...)
{
    // Build log string
	char buffer[255];
    va_list arglist;
    va_start( arglist, _format );
    vsnprintf(buffer, sizeof(buffer), _format, arglist); // store into buffer
	va_end( arglist );

	// Print log only if verbosity level allows it
	if ( _verbosityLevel <= Log::VerbosityLevel )
    {
        if( _type == LogType::Error )
            std::cout << RED "ERR " RESET << buffer;
        else if( _type == LogType::Warning )
            std::cout << MAGENTA "WRN " RESET << buffer;
        else
            std::cout << "MSG " << buffer;
    }

	// Store type and buffer in history
	Logs.push_back( {_type, _verbosityLevel, buffer} );
}
