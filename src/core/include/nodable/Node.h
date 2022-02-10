#pragma once

// libs
#include <observe/event.h>

// std
#include <string>
#include <memory>
#include <algorithm>

// Nodable
#include <nodable/Nodable.h>
#include <nodable/Properties.h>
#include <nodable/Component.h>
#include <nodable/Properties.h>

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
         * @brief Slots is a container with limited size
         * TODO: make it generic to use it with Wires too.
         */
        template<typename T>
        struct Slots
        {
            Slots(T _parent, size_t _max_count = std::numeric_limits<size_t>::max() ): m_parent( _parent ), m_max_count(_max_count) {}

            std::vector<T>&       get_data() { return m_slots; }
            const std::vector<T>& get_data() const { return m_slots; }
            auto        begin() { return m_slots.begin(); }
            auto        end() { return m_slots.end(); }
            void        set_max_count(int _count) { m_max_count = _count;}
            int         get_max_count() { return m_max_count;}
            T           back() { return m_slots.back(); }

            void remove(T _node)
            {
                auto found = std::find(m_slots.begin(), m_slots.end(), _node);
                m_slots.erase(found);
                m_on_removed.emit(_node);
            }

            void add(T _node)
            {
                NODABLE_ASSERT(m_slots.size() < m_max_count);
                m_slots.push_back(_node );
                m_on_added.emit(_node);
            }

            bool        accepts()const { return m_slots.size() < m_max_count; }

            T   get_first_or_nullptr()
            {
                return m_slots.empty() ? nullptr : m_slots[0];
            }

            bool        empty() const { return m_slots.empty(); }
            size_t      size() const { return m_slots.size(); }
            T          operator[](size_t _index)const { return m_slots[_index]; }

            observe::Event<T> m_on_added;
            observe::Event<T> m_on_removed;
        private:
            T          m_parent;
            size_t      m_max_count;
            std::vector<T> m_slots;
        };

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
        Slots<Node*>&        output_slots(){ return m_outputs; };
        Slots<Node*>&        successor_slots() { return m_successors; }
        Slots<Node*>&        predecessor_slots() { return m_predecessors; }

        bool                 needs_to_be_deleted() const { return m_needs_to_be_deleted; }
        void                 flag_for_deletion(){ m_needs_to_be_deleted = true;}

		const char*          get_label()const;
        const char*          get_short_label()const;
		void                 set_label(const char*);
		void                 set_label(std::string);
        void                 set_short_label(const char *);

		void                 add_wire(Wire*);
		void                 remove_wire(Wire*);
		Wires&               get_wires();
		int                  get_input_wire_count ()const;
		int                  get_output_wire_count()const;

		void                 set_dirty(bool _value = true);
		bool                 is_dirty()const;

		virtual UpdateResult update();
        virtual bool         eval() const;

        const InvokableOperator* get_connected_operator(const Member* _localMember); // TODO: weird, try to understand why I needed this
        bool                 has_wire_connected_to(const Member *_localMember);

        template<class T> inline T*       as() { return Reflect::cast_pointer<T>(this); }
        template<class T> inline const T* as()const { return Reflect::cast_pointer<const T>(this); }
        template<class T> inline bool     is()const { return as<T>() != nullptr; }

        Properties*          props() { return &m_props; }
        const Properties*    props()const { return &m_props; }

        Member*              get_this_member()const { return props()->get(THIS_MEMBER_NAME);}

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

			Reflect::Class* desired_class = T::Get_class();

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

        observe::Event<Node*, Relation_t> m_on_relation_added;
        observe::Event<Node*, Relation_t> m_on_relation_removed;

        static constexpr const char* VALUE_MEMBER_NAME = "value";
        static constexpr const char* THIS_MEMBER_NAME = "__this__";
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
		Wires              m_wires;
        Slots<Node*>       m_inputs;
        Slots<Node*>       m_outputs;

		REFLECT(Node)
    };
}
