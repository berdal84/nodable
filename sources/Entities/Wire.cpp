#include "Wire.h"
#include "WireView.h"
#include "Variable.h"
#include <algorithm> // for std::find()

using namespace Nodable;

std::vector<Wire*> Wire::s_wires;

Wire::Wire()
{
	s_wires.push_back(this);
}

Wire::~Wire()
{
	s_wires.erase(std::find(s_wires.begin(), s_wires.end(), this));
}

void Wire::setSource(Value* _source)
{
	source     = _source;
	transmitData();
}

void Wire::setTarget(Value* _target)
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