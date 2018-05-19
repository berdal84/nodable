#pragma once

#include "Nodable.h"            // for constants and forward declarations
#include "Value.h"
#include "Wire.h"
#include <string>
#include <vector>
#include <map>
#include <memory>               // for unique_ptr

#define NODE_DEFAULT_INPUT_NAME "default"
#define NODE_DEFAULT_OUTPUT_NAME "default"

namespace Nodable{

	typedef std::map<std::string, Value*> Members;
	typedef std::vector<Wire*>            Wires;

	/* Base class for all Nodes */
	class Node{
	public:
		Node();
		virtual ~Node();

		/* Return the parent container of this node. Could be nullptr if this node is a root. */
		Node_Container*     getParent         ()const;

		/* Return all members of this node */
		const Members&      getMembers        ()const;

		/* Return a member identified by its name */
		Value*              getMember         (const std::string& _name)const;
		Value*              getMember         (const char* _name)const;

		const char*         getLabel          ()const;
		NodeView*           getView           ()const;

		/* Adds a new member identified by its _name. */
		void                addMember         (const char* _name, Type_ _type = Type_Unknown);		
		
		/* Set a new _value to the member _name.
		Side effect : set dirty all nodes connected directly or inderiectly to one of its outputs.*/
		template<typename T>
		void                setMember         (const char* _name, T _value);

		/* Set a new parent container */
		void                setParent         (Node_Container* _container);

		/* Update the label of the node. The label is not unique. */
		virtual void        updateLabel       (){};
		void                setLabel          (const char*);
		void                setLabel          (std::string);

		/* Update the state of this node. Call this once per frame */
		bool                update            ();

		/* derived classes could override this to perform specific computations */
		virtual bool        eval              ();

		Wires&              getWires          ();
		int                 getInputWireCount ()const;
		int                 getOutputWireCount()const;

		/* Force this node to be evaluated at the next update() call */
		void                setDirty          (bool _value);

		bool                isDirty           ()const;

		/* Connect _from._fromOuputName with _to._toInputName.
		the connection is oriented. */
		static void         Connect           (Node* /*_from*/, Node* /*_to*/, const char* _fromOutputName = NODE_DEFAULT_OUTPUT_NAME, const char* _toInputName = NODE_DEFAULT_INPUT_NAME);
	
		/* Disconnect (and delete) a wire. This method is the opposite of Node::Connect.*/
		static void         Disconnect        (Wire* _wire);
	private:
		Members                   members;
		Node_Container*           parent  = nullptr;
		std::string               label   = "Node";
		std::unique_ptr<NodeView> view;
		bool                      dirty   = false;   // when is true -> needs to be evaluated.
		Wires                     wires;             // contains all wires connected to or from this node.
	};

	template<typename T>
	void Node::setMember      (const char* _name, T _value)
	{
		members[std::string(_name)]->setValue(_value);
		setDirty(true);
		updateLabel();	
	}
}
