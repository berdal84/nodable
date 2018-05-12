#pragma once

#include "Nodable.h"            // for constants and forward declarations
#include "Node_Value.h"
#include <string>
#include <vector>
#include <map>

#define NODE_DEFAULT_INPUT_NAME "default"
#define NODE_DEFAULT_OUTPUT_NAME "default"

namespace Nodable{

	typedef std::map<std::string, Node_Value> Members;
	typedef std::vector<Wire*>                Wires;

	/* Base class for all Nodes */
	class Node{
	public:
		Node();
		virtual ~Node();

		Node_Container*   getParent      ()const;
		const Members&    getMembers     ()const;
		const Node_Value& getMember(const std::string& _name)const;
		const Node_Value& getMember(const char* _name)const;
		const char*       getLabel       ()const;
		NodeView*         getView        ()const;

		void              addMember      (const char* _name, Type_ _type = Type_Unknown);
		
		template<typename T>
		void              setMember      (const char* _name, T _value);

		void              setParent      (Node_Container* _container);
		virtual void      updateLabel    (){};
		void              setLabel       (const char*);
		void              setLabel       (std::string);

		virtual bool        evaluate     ();

		void                updateWires  ();
		void                addWire      (Wire*);
		void                removeWire   (Wire*);
		Wires&              getWires     ();
		int                 getInputWireCount()const;
		int                 getOutputWireCount()const;
		bool                isDirty()const{return dirty;}
		static void         Connect        (Node* /*_from*/, Node* /*_to*/, const char* _fromOutputName = NODE_DEFAULT_OUTPUT_NAME, const char* _toInputName = NODE_DEFAULT_INPUT_NAME);
	private:
		Members members;
		Node_Container*     parent  = nullptr;
		std::string         label   = "Node";
		NodeView*           view    = nullptr;
		bool                dirty   = true;
		Wires               wires;
	};

	template<typename T>
	void Node::setMember      (const char* _name, T _value)
	{
		members[std::string(_name)].setValue(_value);
		updateLabel();
		dirty = true;
	}
}
