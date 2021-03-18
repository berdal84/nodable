#pragma once

#include <map>
#include <string>
#include <functional>

#include "mirror.h"
#include "Nodable.h"     // forward declarations
#include "Component/NodeView.h"   	 // base class

namespace Nodable{

	typedef std::pair<std::string, std::function<Node*(void)>> ContextualMenuItem;

	class GraphNodeView: public NodeView {
	public:
	    GraphNodeView();
		virtual ~GraphNodeView(){};
		virtual bool update() override {return true;} ;
		bool    draw() override ;
		void    addContextualMenuItem(std::string _category, std::string _label, std::function<Node*(void)> _lambda);
	private:
        [[nodiscard]] GraphNode* getGraphNode() const;
		std::multimap<std::string, ContextualMenuItem> contextualMenus;

		MIRROR_CLASS(GraphNodeView)
		(
			MIRROR_PARENT(NodeView)
        );

    };
}