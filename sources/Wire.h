#pragma once 

#include "Nodable.h"
#include "Node.h"

namespace Nodable
{
	class Wire : public Node
	{
	public:
		
		enum State_
		{
			State_Disconnected,
			State_Connected,
			State_COUNT
		};

		void        setSource    (Member*);
		void        setTarget    (Member*);

		State_      getState     ()const{return state;}
		Member*     getSource    ()const{return source;}
		Member*     getTarget    ()const{return target;}
		WireView*   getView      ()const{return getComponent<WireView>("view");}

	private:
		/* update this->state according to this->source and this->target values */
		void        updateState();

		Member*     source       = nullptr;
		Member*     target       = nullptr;
		State_      state        = State_Disconnected;
	};
}