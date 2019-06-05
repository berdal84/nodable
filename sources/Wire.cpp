#include "Wire.h"
#include "WireView.h"
#include "Variable.h"
#include <algorithm> // for std::find()

using namespace Nodable;

void Wire::setSource(Member* _source)
{
	source     = _source;
	updateState();
}

void Wire::setTarget(Member* _target)
{
	target     = _target;
	updateState();
}

void Wire::updateState()
{
	if ( target != nullptr && source != nullptr)
		state = State_Connected;		
	else
		state = State_Disconnected;
}