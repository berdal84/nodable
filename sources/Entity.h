#pragma once

#include "Nodable.h"            // for constants and forward declarations
#include "Object.h"
#include "Member.h"
#include <string>
#include <memory>               // for unique_ptr

namespace Nodable{
	/* Base class for all Nodes */
	class Entity : public Object
	{
	public:
		Entity(){setMember("__class__", "Entity");};
		~Entity();

		/* Return the parent container of this node. Could be nullptr if this node is a root. */
		Container*     getParent         ()const;

		const char*         getLabel          ()const;
		
		/* Component related methods */
		void                addComponent      (const std::string&  /*_componentName*/, Component* /* _component */);
		bool                hasComponent      (const std::string&  /*_componentName*/)const;
		Component*          getComponent      (const std::string&  /*_componentName*/)const;
		const Components&   getComponents     ()const{return components;}
		void                removeComponent   (const std::string& /* _componentName */);
		
		/* Set a new parent container */
		void                setParent         (Container* _container);

		/* Update the label of the node. The label is not unique. */
		virtual void        updateLabel       (){};
		void                setLabel          (const char*);
		void                setLabel          (std::string);

		/* Update the state of this node. Call this once per frame */
		bool                update            ();

		void                addWire           (Wire*);
		void                removeWire        (Wire*);
		Wires&              getWires          ();
		int                 getInputWireCount ()const;
		int                 getOutputWireCount()const;

		/* Force this node to be evaluated at the next update() call */
		void                setDirty          (bool _value = true);

		/* return true if this node needs to be updated, either false */
		bool                isDirty           ()const;

		/* Set the node dirty and update its label */
		void                onMemberValueChanged(const char* _name)override;

		/* Connect _from._fromOuputName with _to._toInputName.
		the connection is oriented. */
		static void         Connect            (Wire* /*_wire*/, Member* /*_from*/, Member* /*_to*/);
	
		/* Disconnect a wire. This method is the opposite of Entity::Connect.*/
		static void         Disconnect        (Wire* _wire);
	private:
		Components                components;
		Container*                parent  = nullptr;
		std::string               label   = "Entity";
		bool                      dirty   = false;   // when is true -> needs to be evaluated.
		Wires                     wires;             // contains all wires connected to or from this node.
	};
}
