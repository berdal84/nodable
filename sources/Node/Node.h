#pragma once
#include <string>
#include <memory>               // for unique_ptr

#include "Nodable.h"            // for constants and forward declarations
#include "Object.h"
#include "Member.h"
#include "Component.h"
#include "Compound.h"

namespace Nodable{
	
	/*
		The role of this class is to provide connectable Objects as Nodes.

		A node is an Object (composed by Members) but here they can be linked together in 
		order to create graphs.

		Every Node has a parentContainer (cf. Container class).
		All nodes are built from a Container, which first create an instance of this class (or derived) and then
		add some components (cf. Component and derived classes) on it.
	*/
	class Node : public Object, public Compound
	{
	public:
		Node(std::string /* label */ = "UnnamedNode");
		~Node();

		/* Get parent container of this node.
		   note: Could be nullptr only if this node is a root. */
		Container* getParentContainer()const;

		/* Set the parent container of this node */
		void setParentContainer(Container*);

		/* Get the label of this Node */
		const char* getLabel()const;		
		

		/* Update the label of the node.
		   note: a label is not unique. */
		virtual void        updateLabel       (){};

		/* Set a label for this Node */
		void                setLabel          (const char*);

		/* Set a label for this Node */
		void                setLabel          (std::string);

		/* Update the state of this node.
		   note: this will be called every frame automatically */
		virtual bool        update            ();

		/* Adds a new wire related* to this node. (* connected to one of the Node Member)
		   note: a node can connect two Members (cf. Wire class) */
		void                addWire           (Wire*);

		/* Removes a wire from this Node */
		void                removeWire        (Wire*);

		/* Get wires related* to this node. (* connected to one of the Node Member) */
		Wires&              getWires          ();

		/* Get the input connectedd wire count. */
		int                 getInputWireCount ()const;

		/* Get the output connected wire count. */
		int                 getOutputWireCount()const;

		/* Force this node to be evaluated at the next update() call */
		void                setDirty          (bool _value = true);

		/* return true if this node needs to be updated and false otherwise */
		bool                isDirty           ()const;

		/* Create an oriented edge (Wire) between two Members */
		static Wire* Connect(Member* /*_from*/, Member* /*_to*/);
	
		/* Disconnect a wire. This method is the opposite of Node::Connect.*/
		static void Disconnect(Wire* _wire);

	private:
		/* Will be called automatically on member value changes */
		void onMemberValueChanged(const char* _name)override;
		
		Container*                parentContainer;
		std::string               label;
		bool                      dirty;   // when is true -> needs to be evaluated.
		Wires                     wires;             // contains all wires connected to or from this node.
	
	public:
		MIRROR_CLASS(Node)(
			MIRROR_PARENT(Object)
		);	
	};
}
