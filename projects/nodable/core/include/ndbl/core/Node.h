#pragma once

// libs
#include <observe/event.h>

// std
#include <string>
#include <memory>
#include <algorithm>

#include "fw/core/assertions.h"
#include <fw/core/reflection/reflection>
#include "fw/core/types.h"

#include <ndbl/core/Component.h>
#include <ndbl/core/DirectedEdge.h>
#include <ndbl/core/PropertyGrp.h>
#include <ndbl/core/Slots.h>
#include <ndbl/core/constants.h>

namespace ndbl {

    // forward declarations
    class GraphNode;

    /**
     * Distinguish between all possible update result
     */
    enum class UpdateResult
    {
        Success_NoChanges,
        Success_WithChanges,
        Failed
    };

	/**
		The role of this class is to provide connectable Objects as Nodes.

		A node is an Object (composed by Properties) that can be linked together in
		order to create graphs.

		Every Node has a parent GraphNode. All nodes are built from a GraphNode, which first create an instance of this class (or derived) and then
		add some Component on it.
	*/
	class Node
	{
	public:
        using Components = std::map<std::string, Component*>;
	    /**
	     * Create a new Node
	     * @param _label
	     */
		explicit Node(std::string  _label = "UnnamedNode");
        Node (const Node&) = delete;
        Node& operator= (const Node&) = delete;
		virtual ~Node();

		virtual Node*        get_parent()const { return m_parent; }
        virtual void         set_parent(Node*);
        Slots<Node*>&        children_slots() { return m_children; }
        inline GraphNode*    get_parent_graph()const { return m_parent_graph; }
        void                 set_parent_graph(GraphNode*);
        Slots<Node*>&        inputs() { return m_inputs; };
        const Slots<Node*>&  inputs() const{ return m_inputs; };
        Slots<Node*>&        outputs() { return m_outputs; };
        const Slots<Node*>&  outputs() const { return m_outputs; };
        Slots<Node*>&        successors() { return m_successors; }
        const Slots<Node*>&  successors()const { return m_successors; }
        Slots<Node*>&        predecessors() { return m_predecessors; }
        bool                 flagged_to_delete() const { return m_flagged_to_delete; }
        void                 flag_to_delete(){ m_flagged_to_delete = true;}
        void                 set_name(const char *_label);
        const char*          get_name()const;
		void                 add_edge(const DirectedEdge*);
		void                 remove_edge(const DirectedEdge*);
        size_t               incoming_edge_count()const;
        size_t               outgoing_edge_count()const;
		void                 set_dirty(bool _value = true);
		bool                 is_dirty()const;
        const fw::iinvokable* get_connected_invokable(const Property *each_edge); // TODO: weird, try to understand why I needed this
        bool                 is_connected_with(const Property *_localProperty);

        template<class T> inline T*       as() { return fw::cast<T>(this); }
        template<class T> inline const T* as()const { return fw::cast<const T>(this); }
        template<class T> inline bool     is()const { return fw::cast<const T>(this) != nullptr; }

        PropertyGrp *          props() { return &m_props; }
        const PropertyGrp *    props()const { return &m_props; }
        Property *             get_this_property()const { return props()->get(k_this_property_name);}

		 /**
		  * Add a component to this Node
		  * Check this Node has no other Component of the same type using Node::hasComponent<T>().
		  * @tparam T
		  * @param _component
		  */
		template<typename T, typename... Args>
		T* add_component(Args... args)
		{
			static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");
            auto component = new T(args...);
            component->set_owner(this);
			m_components.emplace(std::make_pair(fw::type::get<T>().get_name(), component));
            return component;
		}

		 /**
		  * Ask if this Node has a Component with type T.
		  * @tparam T must be Component derived.
		  * @return true if this node has the component specified by it's type T.
		  */
		template<typename T>
		[[nodiscard]] bool has()const
		{
			return get<T>();
		}

		/**
		 * Get all components of this Node
		 */
		[[nodiscard]] inline const Components& get_components()const
		{
			return m_components;
		}

		 /**
		  * Delete a component of this node by specifying its type.
		  * @tparam T must be Component derived.
		  */
		template<typename T>
		void delete_component()
		{
			static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");
			auto name = T::Get_class()->get_name();
			auto component = get<T>();
			m_components.erase(name);
			delete component;
		}

		 /**
		  *  Get a Component by type.
		  * @tparam T must be Component derived.
		  * @return a T pointer.
		  */
		template<typename T>
		T* get()const
		{
            static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");

		    if ( m_components.empty() )
            {
                return nullptr;
            }

            fw::type desired_class = fw::type::get<T>();

			// Search with class name
			{
				auto it = m_components.find( desired_class.get_name() );
				if (it != m_components.end())
				{
					return static_cast<T*>(it->second);
				}
			}

			// Search for a derived class
			for (const auto & [name, component] : m_components)
			{
				if ( component->get_type().is_child_of(desired_class) )
				{
					return static_cast<T*>(component);
				}
			}

			return nullptr;
		};

        template<class T>
		static void get_components(const std::vector<Node*>& inNodes, std::vector<T*>& outComponents)
        {
		    // ensure a single alloc
		    outComponents.reserve(inNodes.size());

            for (auto eachNode : inNodes)
                if (T* view = eachNode->get<T>())
                    outComponents.push_back(view);
        }

        size_t delete_components();

        observe::Event<Node*, Edge_t> on_edge_added;
        observe::Event<Node*, Edge_t> on_edge_removed;
        observe::Event<Node*>         on_name_change;

        template<typename T>
        T convert_value_to() const
        {
            const Property * result_node_value = m_props.get(k_value_property_name);
            return result_node_value->get_variant()->convert_to<T>();
        }
        template<typename T>
        T value_as() const
        {
            const Property * result_node_value = m_props.get(k_value_property_name);
            return (T)*result_node_value->get_variant();
        }

	protected:
        PropertyGrp        m_props;
		Components         m_components;
        Node*              m_parent;
        Slots<Node*>       m_successors;
        Slots<Node*>       m_predecessors;
        Slots<Node*>       m_children;
        bool               m_flagged_to_delete;

    private:
		GraphNode*         m_inner_graph;
		GraphNode*         m_parent_graph;
		std::string        m_name;
		bool               m_dirty;
        std::set<const DirectedEdge*> m_edges;
        Slots<Node*>       m_inputs;
        Slots<Node*>       m_outputs;

		REFLECT_BASE_CLASS()

    };
}
