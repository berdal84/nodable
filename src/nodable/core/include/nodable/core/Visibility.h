#pragma once
namespace ndbl
{
	/**
		The role of this enum class is to distinguish between visibility states
		This enum is used a lot into class Property and NodeView
	*/
	enum class Visibility
	{
		Always,
		// OnlyWhenUncollapsed,
		Hidden,
		Default             = Always
	};
}
