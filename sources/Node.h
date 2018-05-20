#pragma once

#include "Nodable.h"            // for constants and forward declarations
#include "Object.h"
#include "Component.h"
#include "Value.h"
#include <string>
#include <memory>               // for unique_ptr

#define NODE_DEFAULT_INPUT_NAME "default"
#define NODE_DEFAULT_OUTPUT_NAME "default"

namespace Nodable{
	/* Base class for all Nodes */
	class Node : public Object
	{
	public:
		Node();
		virtual ~Node();

		/* Return the parent container of this node. Could be nullptr if this node is a root. */
		Node_Container*     getParent         ()const;

		const char*         getLabel          ()const;
		
		/* Component related methods */
		void                addComponent      (const std::string&  /*_componentName*/, Component* /* _component */);
		bool                hasComponent      (const std::string&  /*_componentName*/)const;
		Component*          getComponent      (const std::string&  /*_componentName*/)const;
		const Components&   getComponents     ()const{return components;}

		/* Set a new parent container */
		void                setParent         (Node_Container* _container);

		/* Update the label of the node. The label is not unique. */
		virtual void        updateLabel       (){};
		void                setLabel          (const char*);
		void                setLabel          (std::string);

		/* Update the state of this node. Call this once per frame */
		bool                update            ();

		Wires&              getWires          ();
		int                 getInputWireCount ()const;
		int                 getOutputWireCount()const;

		/* Force this node to be evaluated at the next update() call */
		void                setDirty          (bool _value);

		bool                isDirty           ()const;

		/* Set the node dirty and update its label */
		void                onMemberValueChanged(const char* _name)override;

		/* Connect _from._fromOuputName with _to._toInputName.
		the connection is oriented. */
		static void         Connect            (Wire* /*_wire*/, Node* /*_from*/, Node* /*_to*/, const char* _fromOutputName = NODE_DEFAULT_OUTPUT_NAME, const char* _toInputName = NODE_DEFAULT_INPUT_NAME);
	
		/* Disconnect (and delete) a wire. This method is the opposite of Node::Connect.*/
		static void         Disconnect        (Wire* _wire);
	private:
		Components                components;
		Node_Container*           parent  = nullptr;
		std::string               label   = "Node";
		bool                      dirty   = false;   // when is true -> needs to be evaluated.
		Wires                     wires;             // contains all wires connected to or from this node.
	};
}
