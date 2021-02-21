#pragma once

#include "Nodable.h"     // forward declarations
#include "View.h"   	 // base class
#include <mirror.h>
#include <map>
#include <string>
#include <functional>

namespace Nodable{

	typedef std::pair<std::string, std::function<Node*(void)>> ContextualMenuItem;

	class GraphNodeView: public View{
	public:
		virtual ~GraphNodeView(){};
		virtual bool update(){return true;};
		bool    draw();
		void    addContextualMenuItem(std::string _category, std::string _label, std::function<Node*(void)> _lambda);
	private:
        GraphNode* getGraphNode() const;
		std::multimap<std::string, ContextualMenuItem> contextualMenus;

		MIRROR_CLASS(GraphNodeView)
		(
			MIRROR_PARENT(View)
        );

    };
}