#pragma once

#include "Nodable.h"            // for constants and forward declarations
#include "vector"
#include <string>
#include "stdlib.h"		// for size_t
#include <imgui.h>

#define NODE_DEFAULT_INPUT_NAME "default"
#define NODE_DEFAULT_OUTPUT_NAME "default"

namespace Nodable{


	/* Base class for all Nodes */
	class Node{
	public:
		Node();
		~Node();
		
		void              begin          ();
		virtual void      draw           ();
		void              end            ();

		Node_Container*   getParent      ()const;		
		Node_Variable*    getInput       (const char* _name = NODE_DEFAULT_INPUT_NAME)const;
		Node_Variable*    getOutput      (const char* _name = NODE_DEFAULT_OUTPUT_NAME)const;	
		Node_Variable*    getMember      (const char* _name)const;
		const char*       getLabel       ()const;
		ImVec2            getPosition    ()const;
		ImVec2            getInputPosition()const;
		ImVec2            getOutputPosition()const;

		void              setInput       (Node*, const char* _name = NODE_DEFAULT_INPUT_NAME);
		void              setOutput      (Node*, const char* _name = NODE_DEFAULT_OUTPUT_NAME);
		void              setMember      (Node*, const char* _name);
		void              setParent      (Node_Container* _container);
		void              setLabel       (const char*);
		void              setLabel       (std::string);
		void              setPosition    (ImVec2);

		static void       ArrangeRecursive  (Node*, ImVec2 _position);
		static ImVec2     s_cameraPosition;
	private:
		Node_Container* inputs;
		Node_Container* outputs;		
		Node_Container* members;
		Node_Container* parent = nullptr;
		std::string     label  = "Node";
		ImVec2          position = ImVec2(50.0f, 50.0f);
		ImVec2          size = ImVec2(170.0f, 40.0f);
		float           opacity = 0.0f;
		bool            visible = true;
		bool            showDetails = false;
	};
}
