#pragma once

#include "Nodable.h"     // forward declarations
#include "View.h"   	 // base class


namespace Nodable{

	class ContainerView: public View{
	public:
		virtual ~ContainerView(){};
		bool    draw();
	private:
		void constraintToBeVisible(NodeView*)const;
	};
}