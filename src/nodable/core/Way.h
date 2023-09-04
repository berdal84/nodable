#pragma once
#include "fw/core/reflection/reflection"

namespace ndbl
{
	enum class Way : u8_t
	{
		None    = 0,
		In      = 1 << 0,
		Out     = 1 << 1,
		InOut   = In | Out,
		Default = None
	};

    R_ENUM(Way)
    R_ENUM_VALUE(None)
    R_ENUM_VALUE(In)
    R_ENUM_VALUE(Out)
    R_ENUM_VALUE(InOut)
    R_ENUM_END
}
