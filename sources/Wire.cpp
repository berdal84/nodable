#include "Wire.h"
#include "WireView.h"
#include "Variable.h"
#include <algorithm> // for std::find()

using namespace Nodable;

void Wire::setSource(Member* _source)
{
	source     = _source;
	transmitData();
}

void Wire::setTarget(Member* _target)
{
	target     = _target;
	transmitData();
}

void Wire::transmitData()
{

	if ( target != nullptr && source != nullptr)
	{
		state = State_Connected;
		target->setValue(source);
	}else{
		state = State_Disconnected;
	}
}