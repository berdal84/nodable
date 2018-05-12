#include "Wire.h"
#include "Node.h"
#include "WireView.h"
#include "Node_Variable.h"
#include <algorithm> // for std::find()

using namespace Nodable;

std::vector<Wire*> Wire::s_wires;

Wire::Wire()
{
	s_wires.push_back(this);
	view = new WireView(this);
}

Wire::~Wire()
{
	if (source != nullptr)
		source->removeWire(this);
	if (target != nullptr)
		target->removeWire(this);
	s_wires.erase(std::find(s_wires.begin(), s_wires.end(), this));

	delete view;
}

void Wire::setSource(Node* _node, const char* _slotName)
{
	source     = _node;
	sourceSlot = _slotName;
}

void Wire::setTarget(Node* _node, const char* _slotName)
{
	target     = _node;
	targetSlot = _slotName;
}

void Wire::transmitData()
{

	if ( target != nullptr && source != nullptr)
	{
		state = State_Connected;

		// evaluate the source node before to transmit data to the target
		source->evaluate();
		target->setMember(targetSlot.c_str(), source->getMember(sourceSlot) );
	}else{
		state = State_Disconnected;
	}

}