#pragma once

#include "Nodable.h" // for constants and forward declarations
#include <imgui.h>   // for ImVec2
#include <string>

namespace Nodable{

	enum DrawMode_
	{		
		DrawMode_AsWindow  = 0,
		DrawMode_AsGroup   = 1,
		DrawMode_Default   = DrawMode_AsGroup
	};

	class NodeView{
	public:
		NodeView(Node* _node);
		~NodeView();		

		void              draw                ();
		void              update              ();

		ImVec2            getPosition         ()const;
		ImVec2            getInputPosition    ()const;
		ImVec2            getOutputPosition   ()const;
		Node*             getNode             ()const;

		void              setVisible          (bool);

		bool              isHovered           ()const;
		bool              isDragged           ()const;
		void              setPosition         (ImVec2);
		void              translate           (ImVec2);
		void              arrange             ();
		
		static void       ArrangeRecursive    (NodeView*, ImVec2 _position = ImVec2(1400.0f, 200.0f));
		static void       SetSelected         (NodeView*);
		static NodeView*  GetSelected         ();
		static bool       IsSelected          (NodeView*);
	private:
		void              imguiBegin          (DrawMode_ _drawMode = DrawMode_Default);
		void              imguiDraw           (DrawMode_ _drawMode = DrawMode_Default);
		void              imguiEnd            (DrawMode_ _drawMode = DrawMode_Default);
           
        std::string     name;
		Node*           node        = nullptr;
		ImVec2          position    = ImVec2(50.0f, 50.0f);
		ImVec2          size        = ImVec2(170.0f, 40.0f);
		float           opacity     = 0.0f;
		bool            visible     = true;
		bool            showDetails = false;
		bool            hovered     = false;
		bool            dragged     = false;
		ImColor         borderColorSelected = ImColor(1.0f, 1.0f, 1.0f);
		ImColor         borderColor         = ImColor(0.8f, 1.0f, 0.8f);
		static NodeView* s_selected;
	};
}
