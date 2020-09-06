#pragma once
#include <bitset>

namespace Nodable
{
	/*
	  The role of this enum is to distinguish the way
	  to connect a specific connector.
	*/
	typedef enum Way
	{
		Way_None    = 0,
		Way_In      = 1 << 1,
		Way_Out     = 1 << 2,
		Way_InOut   = Way_In | Way_Out,
		Way_Default = Way_None
	} Way;

}
