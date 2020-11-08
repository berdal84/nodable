#pragma once
#include <memory>
#include "mirror.h"
#include "WireView.h"

namespace Nodable
{
    class Member;

	class Wire
	{
	public:

	    Wire() = default;
	    ~Wire() = default;

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
		void        newView();
		WireView*   getView      ()const;

	private:
	    std::unique_ptr<WireView> view;

		/* update this->state according to this->source and this->target values */
		void        updateState();

		Member*     source       = nullptr;
		Member*     target       = nullptr;
		State_      state        = State_Disconnected;
	};
}