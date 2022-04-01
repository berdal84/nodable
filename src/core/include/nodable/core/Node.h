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

namespace Nodable {

    // forward declarations
    class InvokableOperator;
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

        Slots<Node*>&        input_slots() { return m_inputs; };
        const Slots<Node*>&  input_slots() const{ return m_inputs; };
        Slots<Node*>&        output_slots() { return m_outputs; };
        const Slots<Node*>&  output_slots() const { return m_outputs; };
        Slots<Node*>&        successor_slots() { return m_successors; }
        const Slots<Node*>&  successor_slots()const { return m_successors; }
        Slots<Node*>&        predecessor_slots() { return m_predecessors; }
        const Slots<Node*>&  predecessor_slots()const { return m_predecessors; }

        bool                 needs_to_be_deleted() const { return m_needs_to_be_deleted; }
        void                 flag_for_deletion(){ m_needs_to_be_deleted = true;}

		const char*          get_label()const;
        const char*          get_short_label()const;
		void                 set_label(const char*);
		void                 set_label(std::string);
        void                 set_short_label(const char *);

		void                 add_wire(Wire*);
		void                 remove_wire(Wire*);
		WireVec&             get_wires();
		int                  get_input_wire_count ()const;
		int                  get_output_wire_count()const;

		void                 set_dirty(bool _value = true);
		bool                 is_dirty()const;

		virtual UpdateResult update();
        virtual bool         eval() const;

        const InvokableOperator* get_connected_operator(const Member* _localMember); // TODO: weird, try to understand why I needed this
        bool                 has_wire_connected_to(const Member *_localMember);

        template<class T> inline T*       as() { return R::cast_pointer<T>(this); }
        template<class T> inline const T* as()const { return R::cast_pointer<const T>(this); }
        template<class T> inline bool     is()const { return R::cast_pointer<const T>(this) != nullptr; }

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
			std::string name(T::Get_class()->get_name());
			m_components.emplace(std::make_pair(name, _component));
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

			R::Class_ptr desired_class = T::Get_class();

			// Search with class name
			{
				auto it = m_components.find( desired_class->get_name() );
				if (it != m_components.end())
				{
					return static_cast<T*>(it->second);
				}
			}

			// Search for a derived class
			for (const auto & it : m_components)
			{
				Component* each_component = it.second;
				if ( each_component->get_class()->is_child_of(desired_class, false) )
				{
					return static_cast<T*>(each_component);
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
            return result_node_value->convert_to<T>();
        }
        template<typename T>
        T value_as() const
        {
            const Member* result_node_value = m_props.get(k_value_member_name);
            return (T)*result_node_value->get_data();
        }

	protected:
        Properties         m_props;
		Components         m_components;
        Node*              m_parent;
        Slots<Node*>       m_successors;
        Slots<Node*>       m_predecessors;
        Slots<Node*>       m_children;
        bool               m_needs_to_be_deleted;

    private:
		GraphNode*         m_inner_graph;
		GraphNode*         m_parent_graph;
		std::string        m_label;
		std::string        m_short_label;
		bool               m_dirty;
		WireVec            m_wires;
        Slots<Node*>       m_inputs;
        Slots<Node*>       m_outputs;

		R(Node)

    };
}
