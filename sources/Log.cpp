#include "Log.h"

#include <cstdarg> // va_list, va_start, va_end
#include <cstdio>  // vfprintf

void Nodable::internal::Log(const char* _format, ...)
{
	va_list args;
	va_start(args, _format);
	vfprintf(stdout, _format, args);
	va_end(args);
}