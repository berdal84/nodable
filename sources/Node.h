#pragma once

#include "Nodable.h"            // for constants and forward declarations
#include <string>

#define NODE_DEFAULT_INPUT_NAME "default"
#define NODE_DEFAULT_OUTPUT_NAME "default"

namespace Nodable{


	/* Base class for all Nodes */
	class Node{
	public:
		Node();
		virtual ~Node();

		Node_Container*   getParent      ()const;
		Node_Container*   getInputs      ()const;		
		Node_Variable*    getInput       (const char* _name = NODE_DEFAULT_INPUT_NAME)const;
		Node_Container*   getOutputs     ()const;
		Node_Variable*    getOutput      (const char* _name = NODE_DEFAULT_OUTPUT_NAME)const;	
		Node_Container*   getMembers     ()const;
		Node_Variable*    getMember      (const char* _name)const;
		const char*       getLabel       ()const;
		NodeView*         getView        ()const;

		void              setInput       (Node*, const char* _name = NODE_DEFAULT_INPUT_NAME);
		void              setOutput      (Node*, const char* _name = NODE_DEFAULT_OUTPUT_NAME);
		void              setMember      (Node*, const char* _name);
		void              setParent      (Node_Container* _container);
		void              setLabel       (const char*);
		void              setLabel       (std::string);

		static void       Connect        (Node* /*_from*/, Node* /*_to*/, const char* _fromOutputName = NODE_DEFAULT_OUTPUT_NAME, const char* _toInputName = NODE_DEFAULT_INPUT_NAME);
	private:
		Node_Container* inputs;
		Node_Container* outputs;		
		Node_Container* members;
		Node_Container* parent = nullptr;
		std::string     label  = "Node";
		NodeView*       view   = nullptr;
	};
}
