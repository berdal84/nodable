#pragma once

#include "Nodable.h"     // forward declarations
#include "View.h"   	 // base class


namespace Nodable{

	class ContainerView: public View{
	public:
		COMPONENT_CONSTRUCTOR(ContainerView);

		virtual ~ContainerView(){};
		bool    draw();
	};
}