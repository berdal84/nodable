#include "Wire.h"
#include "WireView.h"
#include "Variable.h"
#include <memory> // unique_ptr

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

Nodable::WireView* Nodable::Wire::getView() const
{
	return this->view.get();
}

void Wire::newView() {
    view = std::make_unique<WireView>(this);
}
