#pragma once

#include "Nodable.h"     // forward declarations
#include "View.h"   	 // base class
#include <mirror.h>
#include <map>
#include <string>

namespace Nodable{

	class ContainerView: public View{
	public:
		virtual ~ContainerView(){};
		virtual bool update(){return true;};
		bool    draw();
		void    addContextualMenuItem(std::string _label, std::function<Node*(void)> _lambda);
	private:
		std::map<std::string, std::function<Node*(void)>> contextualMenuItems;
		MIRROR_CLASS(ContainerView)(
			MIRROR_PARENT(View));
	};
}