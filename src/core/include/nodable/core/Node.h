#pragma once

// libs
#include <observe/event.h>

// std
#include <string>
#include <memory>
#include <algorithm>

// Nodable
#include <nodable/core/assertions.h>
#include <nodable/core/types.h>
#include <nodable/core/constants.h>
#include <nodable/core/Properties.h>
#include <nodable/core/Component.h>
#include <nodable/core/Properties.h>
#include <nodable/core/Slots.h>
#include <nodable/core/Edge.h>
#include <nodable/core/reflection/invokable.h>

namespace ndbl {

    // forward declarations
    class GraphNode;
    class iinvokable;

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
        Node (const Node&) = delete;
        Node& operator= (const Node&) = delete;
		virtual ~Node();

		virtual Node*        get_parent()const { return m_parent; }
        virtual void         set_parent(Node*);

        Slots<Node*>&        children_slots() { return m_children; }
        const Slots<Node*>&  children_slots()const { return m_children; }

        inline GraphNode*    get_parent_graph()const { return m_parent_graph; }
        void                 set_parent_graph(GraphNode*);
		GraphNode*           get_inner_graph()const;
		void                 get_inner_graph(GraphNode*);

        Slots<Node*>&        inputs() { return m_inputs; };
        const Slots<Node*>&  inputs() const{ return m_inputs; };
        Slots<Node*>&        outputs() { return m_outputs; };
        const Slots<Node*>&  outputs() const { return m_outputs; };
        Slots<Node*>&        successors() { return m_successors; }
        const Slots<Node*>&  successors()const { return m_successors; }
        Slots<Node*>&        predecessors() { return m_predecessors; }
        const Slots<Node*>&  predecessors()const { return m_predecessors; }

        bool                 flagged_to_delete() const { return m_flagged_to_delete; }
        void                 flag_to_delete(){ m_flagged_to_delete = true;}

        void                 set_label(const char* _label, const char* _short_label = nullptr);
        const char*          get_label()const;
        const char*          get_short_label()const;

		void                 add_wire(Wire*);
		void                 remove_wire(Wire*);
		WireVec&             get_wires();
		int                  get_input_wire_count ()const;
		int                  get_output_wire_count()const;

		void                 set_dirty(bool _value = true);
		bool                 is_dirty()const;

		virtual UpdateResult update();

        const iinvokable*    get_connected_invokable(const Member* _local_member); // TODO: weird, try to understand why I needed this
        bool                 has_wire_connected_to(const Member *_localMember);

        template<class T> inline T*       as() { return cast<T>(this); }
        template<class T> inline const T* as()const { return cast<const T>(this); }
        template<class T> inline bool     is()const { return cast<const T>(this) != nullptr; }

        Properties*          props() { return &m_props; }
        const Properties*    props()const { return &m_props; }

        Member*              get_this_member()const { return props()->get(k_this_member_name);}

		 /**
		  * Add a component to this Node
		  * Check this Node has no other Component of the same type using Node::hasComponent<T>().
		  * @tparam T
		  * @param _component
		  */
		template<typename T>
		void add_component(T* _component)
		{
			static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");
            NODABLE_ASSERT( _component != nullptr );
			m_components.emplace(std::make_pair(type::get<T>().get_name(), _component));
			_component->set_owner(this);
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

			type desired_class = type::get<T>();

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
        [[nodiscard]] size_t get_component_count() const;

        observe::Event<Node*, EdgeType> m_on_relation_added;
        observe::Event<Node*, EdgeType> m_on_relation_removed;

        template<typename T>
        T convert_value_to() const
        {
            const Member* result_node_value = m_props.get(k_value_member_name);
            return result_node_value->get_variant()->convert_to<T>();
        }
        template<typename T>
        T value_as() const
        {
            const Member* result_node_value = m_props.get(k_value_member_name);
            return (T)*result_node_value->get_variant();
        }

	protected:
        Properties         m_props;
		Components         m_components;
        Node*              m_parent;
        Slots<Node*>       m_successors;
        Slots<Node*>       m_predecessors;
        Slots<Node*>       m_children;
        bool               m_flagged_to_delete;

    private:
		GraphNode*         m_inner_graph;
		GraphNode*         m_parent_graph;
		std::string        m_label;
		std::string        m_short_label;
		bool               m_dirty;
		WireVec            m_wires;
        Slots<Node*>       m_inputs;
        Slots<Node*>       m_outputs;

		REFLECT_BASE_CLASS()

    };
}
