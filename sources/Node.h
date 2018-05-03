#pragma once

#include "Nodable.h"            // for constants and forward declarations
#include "vector"
#include <string>
#include "stdlib.h"		// for size_t

#define NODE_DEFAULT_INPUT_NAME "default"
#define NODE_DEFAULT_OUTPUT_NAME "default"

namespace Nodable{


	/* Base class for all Nodes */
	class Node{
	public:
		Node();
		~Node();
		virtual void      draw           (){};
		Node_Container*   getParent      ()const;		
		Node_Variable*    getInput       (const char* _name = NODE_DEFAULT_INPUT_NAME)const;
		Node_Variable*    getOutput      (const char* _name = NODE_DEFAULT_OUTPUT_NAME)const;	
		Node_Variable*    getMember      (const char* _name)const;
		void              setInput       (Node*, const char* _name = NODE_DEFAULT_INPUT_NAME);
		void              setOutput      (Node*, const char* _name = NODE_DEFAULT_OUTPUT_NAME);
		void              setMember      (Node*, const char* _name);
		void              setParent      (Node_Container* _container);
		static void       DrawRecursive  (Node*, std::string _prefix = "");
	private:
		Node_Container* inputs;
		Node_Container* outputs;		
		Node_Container* members;
		Node_Container* parent = nullptr;
	};
}
