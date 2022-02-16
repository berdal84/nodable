#pragma once 

#include <nodable/R.h>

#include <nodable/Nodable.h>
#include <nodable/Node.h>

namespace Nodable
{
	class Wire
	{
	public:

	    Wire(){}
	    ~Wire(){}

		enum State_
		{
			State_Disconnected,
			State_Connected,
			State_COUNT
		};

		void        setSource    (Member*);
		void        setTarget    (Member*);

		Member*     getSource    ()const{return source;}
		Member*     getTarget    ()const{return target;}

	private:
		Member*     source       = nullptr;
		Member*     target       = nullptr;
	};
}