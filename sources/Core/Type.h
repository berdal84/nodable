#pragma once
namespace Nodable
{
	/*
		The role of this enum class is to distinguish between all types that Nodable can handle.
	*/
	enum Type
	{
        Type_Unknown = 0,
        Type_Any,
        Type_Boolean,
        Type_Double,
        Type_String,
        Type_COUNT
	};
}