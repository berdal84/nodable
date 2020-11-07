#pragma once
#include <bitset>

namespace Nodable
{
	/*
	  The role of this enum is to distinguish the way
	  to connect a specific connector.
	*/
	enum class Way: unsigned int
	{
		None,
		In,
		Out,
		InOut,
		Default = Way::None
	};

}
