#pragma once 

#include "Nodable.h"
#include "Entity.h"    // base class
#include <vector>
#include <string>

namespace Nodable
{
	class Wire : public Entity
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
		Member*      getSource    ()const{return source;}
		Member*      getTarget    ()const{return target;}
		WireView*   getView      ()const{return (WireView*)getComponent("view");}

		/* transfert the value from the source to the target */
		void        transmitData();
	private:
		Member*      source       = nullptr;
		Member*      target       = nullptr;
		State_      state        = State_Disconnected;
	};
}