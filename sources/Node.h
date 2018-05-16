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

		Node_Container*     getParent         ()const;
		const Members&      getMembers        ()const;
		Value*              getMember         (const std::string& _name)const;
		Value*              getMember         (const char* _name)const;
		const char*         getLabel          ()const;
		NodeView*           getView           ()const;
		void                addMember         (const char* _name, Type_ _type = Type_Unknown);		
		template<typename T>
		void                setMember         (const char* _name, T _value);
		void                setParent         (Node_Container* _container);
		virtual void        updateLabel       (){};
		void                setLabel          (const char*);
		void                setLabel          (std::string);
		bool                update            ();
		virtual bool        eval              ();
		void                addWire           (Wire*);
		void                removeWire        (Wire*);
		Wires&              getWires          ();
		int                 getInputWireCount ()const;
		int                 getOutputWireCount()const;
		void                setDirty          (bool _value);
		bool                isDirty           ()const{return dirty;}
		static void         Connect           (Node* /*_from*/, Node* /*_to*/, const char* _fromOutputName = NODE_DEFAULT_OUTPUT_NAME, const char* _toInputName = NODE_DEFAULT_INPUT_NAME);
	private:
		Members             members;
		Node_Container*     parent  = nullptr;
		std::string         label   = "Node";
		std::unique_ptr<NodeView> view;
		bool                dirty   = false;
		Wires               wires;
	};

	/* Set member value to _value and transmit data to its outputs */
	template<typename T>
	void Node::setMember      (const char* _name, T _value)
	{
		members[std::string(_name)]->setValue(_value);
		
		// Transmit Data thru wire
		for(auto wire : wires)
		{
			if ( wire->getSource() == this && wire->getSourceSlot() == std::string(_name))
			{
				wire->transmitData();
			}
		}		
		setDirty(true);
		updateLabel();	
	}
}
