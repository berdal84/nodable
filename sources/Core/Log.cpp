#include "Log.h"

#include <cstdarg> // va_list, va_start, va_end
#include <cstdio>  // vfprintf
#include <iostream>
#include <algorithm>

using namespace Nodable;

std::vector<Log::Message> Log::Logs;
Log::Verbosity Log::s_verbosityLevel = Log::Verbosity::Default;

const Log::Message* Log::GetLastMessage()
{
    auto found = std::find_if( Logs.rbegin(), Logs.rend(), [](auto item)-> bool {
        return item.verbosity <= Log::s_verbosityLevel;
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

void Log::SetVerbosityLevel(Verbosity _verbosityLevel)
{
   s_verbosityLevel = _verbosityLevel;
}

void Log::Push(Log::Type _type, Verbosity _verbosityLevel, const char* _format, ...)
{
	// Print log only if verbosity level allows it
	if ( _verbosityLevel <= s_verbosityLevel )
    {
        // Build log string
        char buffer[255];
        va_list arglist;
        va_start( arglist, _format );
        vsnprintf(buffer, sizeof(buffer), _format, arglist); // store into buffer
        va_end( arglist );

        if( _type == Log::Type::Error )
            std::cout << RED "ERR " RESET << buffer;
        else if( _type == Log::Type::Warning )
            std::cout << MAGENTA "WRN " RESET << buffer;
        else
            std::cout << "MSG " << buffer;

        // Store type and buffer in history
        Logs.push_back( {_type, _verbosityLevel, buffer} );

        // Constraint the queue to be size() < 500
        // Erase by chunk of 250
        if ( Logs.size() > 500 )
        {
            Logs.erase(Logs.begin(), Logs.begin() + 250);
        }
    }

}
