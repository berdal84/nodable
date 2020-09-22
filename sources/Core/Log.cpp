#include "Log.h"

#include <cstdarg> // va_list, va_start, va_end
#include <cstdio>  // vfprintf
#include <iostream>

void Nodable::internal::LogMessage(const char* _prefix, const char* _format, ...)
{
	std::cout << _prefix;

	va_list args;
	va_start(args, _format);
	vfprintf(stdout, _format, args);
	va_end(args);

	std::cout << RESET; // reset color
}


