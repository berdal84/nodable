#include "Log.h"

#include <cstdarg> // va_list, va_start, va_end
#include <cstdio>  // vfprintf
#include <iostream>
#include <algorithm>

using namespace Nodable;

std::vector<Log::Message> Log::Logs;
std::map<std::string, Log::Verbosity> Log::s_verbosityLevels;

const Log::Message* Log::GetLastMessage()
{
    return &Log::Logs.back();
}

void Log::SetVerbosityLevel(const std::string& _category, Verbosity _verbosityLevel)
{
   s_verbosityLevels.insert_or_assign(_category, _verbosityLevel );
}

Log::Verbosity Log::GetVerbosityLevel(const std::string& _category)
{
    auto pair = s_verbosityLevels.find(_category);
    if ( pair != s_verbosityLevels.end() )
    {
        return pair->second;
    }
    return Verbosity::Message;
}

void Log::Push(Verbosity _verbosityLevel, const std::string& _category, const char* _format, ...)
{
	// Print log only if verbosity level allows it

	if (  _verbosityLevel <= GetVerbosityLevel(_category) )
    {
        // Build log string
        char buffer[255];
        va_list arglist;
        va_start( arglist, _format );
        vsnprintf(buffer, sizeof(buffer), _format, arglist); // store into buffer
        va_end( arglist );

        if( _verbosityLevel == Log::Verbosity::Error )
            std::cout << RED "ERR " RESET << buffer;
        else if( _verbosityLevel == Log::Verbosity::Warning )
            std::cout << MAGENTA "WRN " RESET << buffer;
        else
            std::cout << "MSG " << buffer;

        // Store type and buffer in history
        Logs.push_back( {_category, _verbosityLevel, buffer} );

        // Constraint the queue to be size() < 500
        // Erase by chunk of 250
        if ( Logs.size() > 500 )
        {
            Logs.erase(Logs.begin(), Logs.begin() + 250);
        }
    }

}
