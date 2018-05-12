#pragma once

#include "Nodable.h" // for constants and forward declarations
#include "View.h"    // base class
#include <imgui.h>   // for ImVec2
#include <string>

namespace Nodable{

	enum DrawMode_
	{		
		DrawMode_AsWindow  = 0,
		DrawMode_AsGroup   = 1,
		DrawMode_Default   = DrawMode_AsGroup
	};

	enum DrawDetail_
	{
		DrawDetail_Simple  = 0,
		DrawDetail_Advanced = 1,
		DrawDetail_Complex  = 2,
		DrawDetail_Default = DrawDetail_Simple
	};

	class NodeView : public View
	{
	public:
		NodeView(Node* _node);
		~NodeView();		

		/* Draw the view at its position into the current window*/
		void              draw                ();

		void              update              ();

		ImVec2            getPosition         ()const;
		ImVec2            getInputPosition    ()const;
		ImVec2            getOutputPosition   ()const;
		Node*             getNode             ()const;

		void              setVisible          (bool);

		bool              isHovered           ()const;

		void              setPosition         (ImVec2);
		void              translate           (ImVec2);

		/* Arrange input nodes recursively while keeping this node position unchanged */
		void              arrange             ();
		
		static void       ArrangeRecursive    (NodeView*, ImVec2 _position = ImVec2(1400.0f, 200.0f));
		static void       SetSelected         (NodeView*);
		static NodeView*  GetSelected         ();
		static bool       IsSelected          (NodeView*);
		static void       SetDragged          (NodeView*);
		static NodeView*  GetDragged          ();
	private:
		void              imguiBegin          ();
		void              imguiDraw           ();
		void              imguiEnd            ();
           
    public:
		static DrawMode_   s_drawMode;
		static DrawDetail_ s_drawDetail;
	private:
        std::string     name                = "UnnamedNode";
		Node*           node                = nullptr;
		ImVec2          position            = ImVec2(-1.0, -1.0f);
		ImVec2          size                = ImVec2(170.0f, 40.0f);
		float           opacity             = 0.0f;
		bool            visible             = true;
		bool            showDetails         = false;
		bool            hovered             = false;

		bool            couldBeArranged     = true;
		float           borderRadius        = 5.0f;
		ImColor         borderColorSelected = ImColor(1.0f, 1.0f, 1.0f);
		
		static NodeView* s_selected;
		static NodeView* s_dragged;		
	};
}
