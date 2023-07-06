#pragma once

#include <observe/event.h>
#include <string>
#include <memory>
#include <algorithm>

#include "fw/core/assertions.h"
#include "fw/core/reflection/reflection"
#include "fw/core/types.h"

#include "Component.h"
#include "Components.h"
#include "DirectedEdge.h"
#include "PropertyGrp.h"
#include "Slots.h"
#include "constants.h"

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

	    /**
	     * Create a new Node
	     * @param _label
	     */
		explicit Node(std::string  _label = "UnnamedNode");
        Node (const Node&) = delete;
        Node& operator= (const Node&) = delete;
		virtual ~Node() {}

		inline Node*         get_parent()const { return m_parent; }
        inline void          set_parent(Node* _node)
        {
            FW_ASSERT(_node != nullptr || this->m_parent != nullptr);
            this->m_parent = _node;
            set_dirty();
        }
        inline Slots<Node*>& children_slots() { return m_children; }
        inline GraphNode*    get_parent_graph()const { return m_parent_graph; }
        inline void          set_parent_graph(GraphNode* _parent_graph)
        {
            FW_ASSERT(this->m_parent_graph == nullptr); // TODO: implement parentGraph switch
            this->m_parent_graph = _parent_graph;
        }

        inline Components&          components() { return m_components; }
        inline Slots<Node*>&        inputs() { return m_inputs; };
        inline const Slots<Node*>&  inputs() const{ return m_inputs; };
        inline Slots<Node*>&        outputs() { return m_outputs; };
        inline const Slots<Node*>&  outputs() const { return m_outputs; };
        inline Slots<Node*>&        successors() { return m_successors; }
        inline const Slots<Node*>&  successors()const { return m_successors; }
        inline Slots<Node*>&        predecessors() { return m_predecessors; }
        inline bool                 flagged_to_delete() const { return m_flagged_to_delete; }
        inline void                 flag_to_delete(){ m_flagged_to_delete = true;}

        inline void set_name(const char *_label)
        {
            m_name = _label;
            on_name_change.emit(this);
        }

        inline const char* get_name()const
        { return m_name.c_str(); }

		inline void add_edge(const DirectedEdge* edge)
        {
            m_edges.insert(edge);
            m_dirty = true;
        }

		void                 remove_edge(const DirectedEdge*);
        size_t               incoming_edge_count()const;
        size_t               outgoing_edge_count()const;
		inline void          set_dirty(bool _value = true) { m_dirty = _value; }
		inline bool          is_dirty()const { return m_dirty; };
        const fw::iinvokable* get_connected_invokable(const Property *each_edge); // TODO: weird, try to understand why I needed this
        bool                 is_connected_with(const Property *_localProperty);

        template<class T> inline T*       as() { return fw::cast<T>(this); }
        template<class T> inline const T* as()const { return fw::cast<const T>(this); }
        template<class T> inline bool     is()const { return fw::cast<const T>(this) != nullptr; }

        inline PropertyGrp*          props() { return &m_props; }
        inline const PropertyGrp*    props()const { return &m_props; }
        inline Property*             get_this_property()const { return m_props.get(k_this_property_name);}

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

        template<class T>
        static void get_components(const std::vector<Node*>& inNodes, std::vector<T*>& outComponents)
        {
            outComponents.reserve(inNodes.size());

            for (auto eachNode : inNodes)
                if (T* view = eachNode->m_components.get<T>())
                    outComponents.push_back(view);
        }

        template<class T>
        inline T* get_component()
        { return m_components.get<T>(); }

        template<class T>
        inline T* get_component() const
        { return m_components.get<T>(); }

        template<class T>
        inline bool has_component() const
        { return m_components.has<T>(); }

        template<typename T, typename... Args>
        T* add_component(Args... args)
        { return m_components.add<T>(args...); }

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
