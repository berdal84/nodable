#pragma once
namespace Nodable
{
	/*
		The role of this enum class is to distinguish between all types that Nodable can handle.
	*/
	enum class Type
	{
	    Unknown,
		Any,
		Boolean,
		Double,
		String,
		COUNT
	};
}