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

		void        setSource    (std::shared_ptr<Member>);
		void        setTarget    (std::shared_ptr<Member>);

		State_      getState     ()const{return state;}
        std::shared_ptr<Member> getSource()const{return source;}
        std::shared_ptr<Member> getTarget()const{return target;}
		void        newView();
		WireView*   getView      ()const;

	private:
	    std::unique_ptr<WireView> view;

		/* update this->state according to this->source and this->target values */
		void        updateState();

        std::shared_ptr<Member> source;
        std::shared_ptr<Member> target;
		State_ state = State_Disconnected;
	};
}