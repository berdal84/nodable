#pragma once

#include "Nodable.h"            // for constants and forward declarations
#include <imgui.h>

namespace Nodable{


	class NodeView{
	public:
		NodeView(Node* _node);
		~NodeView();		

		void              draw                ();
		void              update              ();

		ImVec2            getPosition         ()const;
		ImVec2            getInputPosition    ()const;
		ImVec2            getOutputPosition   ()const;

		void              setPosition         (ImVec2);

		static void       ArrangeRecursive    (NodeView*, ImVec2 _position);

	private:
		void              imguiBegin          ();
		void              imguiDraw           ();
		void              imguiEnd            ();
             
		Node*           node        = nullptr;
		ImVec2          position    = ImVec2(50.0f, 50.0f);
		ImVec2          size        = ImVec2(170.0f, 40.0f);
		float           opacity     = 0.0f;
		bool            visible     = true;
		bool            showDetails = false;
	};
}
