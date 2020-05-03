#pragma once

#include "Nodable.h"     // forward declarations
#include "View.h"   	 // base class
#include <mirror.h>

namespace Nodable{

	class ContainerView: public View{
	public:
		virtual ~ContainerView(){};
		bool    draw();
		MIRROR_CLASS(ContainerView)(
			MIRROR_PARENT(View));
	};
}