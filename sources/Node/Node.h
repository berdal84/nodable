#pragma once

// std
#include <string>
#include <memory>
#include <algorithm>

// Nodable
#include <Core/Nodable.h>
#include <Core/Properties.h>
#include <Component/Component.h>
#include <Node/GraphTraversal.h>
#include "Properties.h"

namespace Nodable{

    // forward declarations
    class Operator;
    class GraphNode;

    /**
     * Distinguish between all possible update result
     */
    enum class UpdateResult
    {
        SuccessWithoutChanges,
        Success,
        Failed,
        Stopped
    };

	/**
		The role of this class is to provide connectable Objects as Nodes.

		A node is an Object (composed by Members) that can be linked together in
		order to create graphs.

		Every Node has a parent GraphNode. All nodes are built from a GraphNode, which first create an instance of this class (or derived) and then
		add some Component on it.
	*/
	class Node
	{
	public:

	    /**
	     * Create a new Node
	     * @param _label
	     */
		explicit Node(std::string  _label = "UnnamedNode");
		virtual ~Node();

		[[nodiscard]] virtual Node*                     getParent()const { return this->m_parent; }
		              virtual void                      setParent(Node* _node);

		[[nodiscard]] virtual std::vector<Node*>&       getChildren() { return this->m_children; }
        [[nodiscard]] virtual const std::vector<Node*>& getChildren()const { return this->m_children; }
		              virtual void                      addChild(Node* _node);
		              virtual void                      removeChild(Node* _node);

        [[nodiscard]] inline GraphNode*                 getParentGraph()const { return this->m_parentGraph; }
                      void                              setParentGraph(GraphNode* _parentGraph);
		[[nodiscard]] GraphNode*                        getInnerGraph()const;
		              void                              setInnerGraph(GraphNode*);

		              void                addInput(Node *_node);
                      void                addOutput(Node *_node);
                      void                removeInput(Node *_node);
                      void                removeOutput(Node *_node);
                      std::vector<Node*>& getInputs();
                      std::vector<Node*>& getOutputs();

                      void                setNext(Node* _node) { this->m_next = _node; };
                      virtual Node*       getNext() { return this->m_next; }

        bool needsToBeDeleted  () const { return m_deletedFlag; }

        /* Set deleted flag on. Will be deleted by its controller next frame */
        void                flagForDeletion   (){ m_deletedFlag = true;}

		/**
		 * Get the label of this Node
		 * @return
		 */
		[[nodiscard]] const char* getLabel()const;

        [[nodiscard]] const char* getShortLabel()const;

		/** Update the label of the node.
		   note: a label is not unique. */
		virtual void updateLabel(){};

		/** Set a label for this Node */
		void setLabel (const char*);

		/** Set a label for this Node */
		void setLabel(std::string);

        void setShortLabel(const char *_label);

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
		
		/** Update the state of this (and only this) node
		 * This WON'T evaluate it */
		virtual UpdateResult update();
        bool eval() const;

		/** Get the operator connected to a given Member */
        const Operator* getConnectedOperator(const Member* _localMember);

        /**
         * Return true is the local member is connected, false otherwise.
         *
         * source (?) ------> target (_localMember)
         *
         * @param _localMember
         * @return
         */
        bool hasWireConnectedTo(const Member *_localMember);

        /** Get the source Member of a given local Member.
         * A source Member is connected to another as a source.
         *
         * source ------> target (_localMember)
         *
         * @param _localMember
         * @return
         */
        Member* getSourceMemberOf(const Member *_localMember);

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
			m_components.emplace(std::make_pair(name, _component));
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
			return m_components;
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
			m_components.erase(name);
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
				auto it = m_components.find(name);
				if (it != m_components.end()) {
					return reinterpret_cast<T*>(it->second);
				}
			}

			// Search for a derived class
			for (const auto & it : m_components) {
				Component* component = it.second;
				if (component->getClass()->isChildOf(c, false)) {
					return reinterpret_cast<T*>(component);
				}
			}

			return nullptr;
		};
        size_t deleteComponents();
        [[nodiscard]] size_t getComponentCount() const;

        template<class T> [[nodiscard]] T* as()
        {
            if( this->getClass()->isChildOf(mirror::GetClass<T>()))
                return reinterpret_cast<T*>(this);
            return nullptr;
        }

        template<class T> [[nodiscard]] const T* as()const
        {
            if( this->getClass()->isChildOf(mirror::GetClass<T>()))
                return reinterpret_cast<const T*>(this);
            return nullptr;
        }

        Properties* getProps() { return &m_props; }

	protected:
        Properties         m_props;
		Components         m_components;
        Node*              m_parent;
        Node*              m_next;
        std::vector<Node*> m_children;
        bool               m_deletedFlag;

    private:
		GraphNode*         m_innerGraph;
		GraphNode*         m_parentGraph;
		std::string        m_label;
		std::string        m_shortLabel;
		bool               m_dirty;
		Wires              m_wires;
		std::vector<Node*> m_inputs;
		std::vector<Node*> m_outputs;

	public:
	    /* use mirror to refect class */
		MIRROR_CLASS(Node)();
    };
}
