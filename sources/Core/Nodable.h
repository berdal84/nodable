
#pragma once
#include <cassert> // for ASSERT and VERIFY
#define NODABLE_VERIFY(expression) assert(expression)

/*
	Asserts
*/

#ifdef _DEBUG
#   define NODABLE_ASSERT(expression) assert(expression)
#else
#   define NODABLE_ASSERT(expression)
#endif

/*
	Forward declarations
*/

namespace Nodable{ 

	// 0 - globals, check cpp for values.
    extern float bezierCurveOutRoundness;
    extern float bezierCurveInRoundness;
    extern float bezierThickness;
	extern float connectorRadius;
    extern bool  displayArrows; 
	extern float nodePadding;
}




