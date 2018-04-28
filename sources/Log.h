#pragma once

#ifdef DEBUG
	#define LOG_DBG(...) Nodable::internal::Log(__VA_ARGS__)
#else
	#define LOG_DBG(...)
#endif

namespace Nodable{	
	namespace internal
	{
		void Log (const char* _format, ...);
	}
}