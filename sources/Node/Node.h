#pragma once
#include <string>
#include <memory>               // for unique_ptr

#include "Nodable.h"            // for constants and forward declarations
#include "Object.h"
#include "Member.h"

namespace Nodable{
	/* Base class for all Nodes */
	class Node : public Object
	{
	public:
		Node();
		~Node();

		/* Return the parentContainer container of this node. Could be nullptr if this node is a root. */
		Container*     getParentContainer         ()const;

		const char*         getLabel          ()const;
		
		/* Component related methods */
		template<typename T>
		void addComponent(const std::string& _componentName, T* _component)
		{
			auto name = T::GetClass()->getName();
			components.insert_or_assign(name, _component);
			_component->setOwner(this);
		}

		template<typename T>
		bool                hasComponent()const
		{
			return getComponent<T>() != nullptr;
		}


		const Components&   getComponents     ()const{return components;}
		void                removeComponent   (const std::string& /* _componentName */);

		template<typename T>
		T* getComponent()const {

			auto c    = T::GetClass();
			auto name = c->getName();

			// Search with class name
			{
				auto it = components.find(name);
				if (it != components.end()) {
					return it->second->as<T>();
				}
			}

			// Search for a derived class
			for (auto it = components.begin(); it != components.end(); it++) {
				Component* component = it->second;
				if (component->getClass()->isChildOf(c, false)) {
					return component->as<T>();
				}
			}

			return nullptr;
		};

		/*
		Component* getComponent(const std::string& _componentName)const {
			return components.at(_componentName);
		};*/

		/* Set a new parentContainer container */
		void                setParentContainer         (Container* _container);

		/* Update the label of the node. The label is not unique. */
		virtual void        updateLabel       (){};
		void                setLabel          (const char*);
		void                setLabel          (std::string);

		/* Update the state of this node. Call this once per frame */
		virtual bool        update            ();

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
	
		/* Disconnect a wire. This method is the opposite of Node::Connect.*/
		static void         Disconnect        (Wire* _wire);

	private:
		Components                components;
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
