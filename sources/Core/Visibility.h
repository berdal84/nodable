#pragma once
namespace Nodable
{
	/*
		The role of this enum class is to distinguish between visibility states
		This enum is used a lot into class Member and NodeView 
	*/
	enum class Visibility
	{
		Always              = 0,
		OnlyWhenUncollapsed = 1,
		Hidden              = 2,
		Default             = Always
	};
}
