#pragma once

// std
#include <string>
#include <memory>

// Nodable
#include <Core/Nodable.h>
#include <Core/Object.h>
#include <Component/Component.h>
#include <Node/NodeTraversal.h>

namespace Nodable{

    class Operator;

    /**
     * Distinguish between all possible update result
     */
    enum class UpdateResult
    {
        SuccessWithoutChanges,
        SuccessWithChanges,
        Failed,
        Stopped
    };

	/**
		The role of this class is to provide connectable Objects as Nodes.

		A node is an Object (composed by Members) that can be linked together in
		order to create graphs.

		Every Node has a parent Container. All nodes are built from a Container, which first create an instance of this class (or derived) and then
		add some Component on it.
	*/
	class Node : public Object
	{
	public:

	    /**
	     * Create a new Node
	     * @param _label
	     */
		explicit Node(std::string  _label = "UnnamedNode");

		~Node() override;

        /**
         * Get parent Container of this Node.
         * @return the parent Container of this Node. Might be nullptr only if this node is a root.
         */
		[[nodiscard]] Container* getParentContainer()const;

		/**
		 * Set a parent Container to this Node.
		 */
		void setParentContainer(Container*);

		/**
		 * Get the inner Container of this Node.
		 * @return a Container, might be nullptr.
		 */
		[[nodiscard]] Container* getInnerContainer()const;

		/**
		 * Set an inner Container to this Node.
		 * @param _container
		 */
		void setInnerContainer(Container* _container);

		/**
		 * Get the label of this Node
		 * @return
		 */
		[[nodiscard]] const char* getLabel()const;

		/** Update the label of the node.
		   note: a label is not unique. */
		virtual void updateLabel(){};

		/** Set a label for this Node */
		void setLabel (const char*);

		/** Set a label for this Node */
		void setLabel(std::string);

		/** Adds a new Wire connected to one of the Node's Members.*/
		void addWire(Wire*);

		/** Removes a wire from this Node
		 * It doesn't mean that the wire is disconnected. */
		void removeWire(Wire*);

		/** Get all wires connected to one of the Node's Members.*/
		Wires& getWires();

		/** Get the input connected Wire count. */
		[[nodiscard]] int getInputWireCount ()const;

		/** Get the output connected Wire count. */
		[[nodiscard]] int getOutputWireCount()const;

		/** Force this node to be evaluated at the next update() call */
		void setDirty(bool _value = true);

		/** return true if this node needs to be updated and false otherwise */
		[[nodiscard]] bool isDirty()const;
		
		/** Update the state of this (and only this) node */
		virtual UpdateResult update();

		/** Get the operator connected to a given Member */
        const Operator* getConnectedOperator(const Member* _localMember);

		/** Connects two Member using a Wire (oriented edge) */
		static Wire* Connect(Member* /*_from*/, Member* /*_to*/);
	
		/** Disconnects a wire. This method is the opposite of Node::Connect.*/
		static void Disconnect(Wire* _wire);

		 /**
		  * Add a component to this Node
		  * Check this Node has no other Component of the same type using Node::hasComponent<T>().
		  * @tparam T
		  * @param _component
		  */
		template<typename T>
		void addComponent(T* _component)
		{
			static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");
			std::string name(T::GetClass()->getName());
			components.emplace(std::make_pair(name, _component));
			_component->setOwner(this);
		}

		 /**
		  * Ask if this Node has a Component with type T.
		  * @tparam T must be Component derived.
		  * @return true if this node has the component specified by it's type T.
		  */
		template<typename T>
		[[nodiscard]] bool hasComponent()const
		{
			static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");
			return getComponent<T>() != nullptr;
		}

		/**
		 * Get all components of this Node
		 */
		[[nodiscard]] inline const Components& getComponents()const
		{
			return components;
		}

		 /**
		  * Delete a component of this node by specifying its type.
		  * @tparam T must be Component derived.
		  */
		template<typename T>
		void deleteComponent()
		{
			static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");
			auto name = T::GetClass()->getName();
			auto component = getComponent<T>();
			components.erase(name);
			delete component;
		}

		 /**
		  *  Get a Component by type.
		  * @tparam T must be Component derived.
		  * @return a T pointer.
		  */
		template<typename T>
		T* getComponent()const
		{
			static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");
			auto c = T::GetClass();
			auto name = c->getName();

			// Search with class name
			{
				auto it = components.find(name);
				if (it != components.end()) {
					return reinterpret_cast<T*>(it->second);
				}
			}

			// Search for a derived class
			for (const auto & it : components) {
				Component* component = it.second;
				if (component->getClass()->isChildOf(c, false)) {
					return reinterpret_cast<T*>(component);
				}
			}

			return nullptr;
		};

	protected:
		Components components;

	private:
		/**
		 * This will be called automatically after a Member value change.
		 * @param _name is the name of the Member that has changed.
		 */
		void onMemberValueChanged(const char* _name)override;

		/** The inner container of this Node. (Recursion)*/
		Container* innerContainer;

		/** The Container that owns this Node */
		Container*                parentContainer;

		/** Label of the Node, will be visible */
		std::string               label;

        /** true means: needs to be evaluated. */
		bool                      dirty;

        /** contains all wires connected to or from this node.*/
		Wires  wires;
	
	public:
		MIRROR_CLASS(Node)(
			MIRROR_PARENT(Object)
		);


        bool hasWireConnectedTo(const Member *_localMember);

        Member *getSourceMemberOf(const Member *_localMember);
    };
}
