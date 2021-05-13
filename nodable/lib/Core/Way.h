#pragma once
#include <bitset>
#include <string>

namespace Nodable
{
	/**
	  The role of this enum is to distinguish the way
	  to connect a specific connector.
	*/
	typedef enum Way: int
	{
		Way_None    = 0,
		Way_In      = 1 << 0,
		Way_Out     = 1 << 1,
		Way_InOut   = Way_In | Way_Out,
		Way_Default = Way_None
	} Way;

	/**
	 * Get the string representation for a given Way_ enum
	 * @param _way
	 * @return
	 */
	std::string WayToString(Way _way);
}
