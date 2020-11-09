#include "Wire.h"
#include "WireView.h"
#include "Variable.h"
#include <memory> // unique_ptr
#include <utility>

using namespace Nodable;

void Wire::setSource(std::shared_ptr<Member> _source)
{
	source     = std::move(_source);
	updateState();
}

void Wire::setTarget(std::shared_ptr<Member> _target)
{
	target     = std::move(_target);
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
