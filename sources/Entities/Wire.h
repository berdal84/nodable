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

		void        setSource    (Value*);
		void        setTarget    (Value*);

		State_      getState     ()const{return state;}
		Value*      getSource    ()const{return source;}
		Value*      getTarget    ()const{return target;}
		WireView*   getView      ()const{return (WireView*)getComponent("view");}

		/* transfert the value from the source to the target */
		void        transmitData();
	private:
		Value*      source       = nullptr;
		Value*      target       = nullptr;
		State_      state        = State_Disconnected;
	};
}