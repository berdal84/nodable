#pragma once

#include <vector>
#include <set>
#include "Token.h"
#include "tools/core/Signals.h"
#include "tools/core/Optional.h"
#include "NodeComponent.h"

namespace ndbl
{
    // forward decl.
    class Node;
    class VariableNode;
    class ScopeView;

    typedef int ScopeFlags;
    enum ScopeFlags_
    {
        ScopeFlags_NONE              = 0,
        ScopeFlags_RECURSE           = 1 << 0,
        ScopeFlags_IF_SAME_NODE      = 1 << 2,
        ScopeFlags_AS_PRIMARY_CHILD  = 1 << 3,
        ScopeFlags_CLEAR_WITH_PARENT = 1 << 4,
        ScopeFlags_INCLUDE_SELF      = 1 << 5,
    };

    class Scope : public NodeComponent
    {
    public:
        DECLARE_REFLECT_override

        SIGNAL(on_reset_parent, Scope*);
        SIGNAL(on_add, Node*);
        SIGNAL(on_remove, Node*);
        SIGNAL(on_change);
        SIGNAL(on_clear);

        Token token_begin = {Token_t::ignore};
        Token token_end   = {Token_t::ignore};

        ~Scope();
        Scope*                         parent() { return m_parent; }
        const Scope*                   parent() const { return m_parent; }
        std::vector<Node*>             leaves();
        void                           clear();
        bool                           empty() const { return m_child.empty(); }
        bool                           empty_ex(ScopeFlags) const;
        VariableNode*                  find_variable_recursively(const std::string& identifier) { return find_var_ex(identifier, ScopeFlags_RECURSE ); } ;
        ScopeView*                     view() const { return m_view; }
        void                           set_view(ScopeView* view) { m_view = view; }
        void                           child_push_back(Node* node) { child_push_back_ex(node, ScopeFlags_RECURSE | ScopeFlags_AS_PRIMARY_CHILD); }
        void                           child_erase(Node* node) { return child_erase_ex(node, ScopeFlags_RECURSE); }
        const std::set<VariableNode*>& variable()const { return m_variable; };
        std::vector<Node*>&            child() { return  m_child; }
        const std::vector<Node*>&      child() const { return m_child; }
        Node*                          first_child() const { return m_child.empty() ? nullptr : *m_child.begin(); };
        Node*                          last_child() const { return m_child.empty() ? nullptr : *m_child.rbegin(); };
        void                           reset_parent(Scope* new_parent);
        bool                           is_partitioned() const { return !m_partition.empty(); }
        bool                           is_partition() const { return m_parent && m_parent->is_partitioned(); }
        std::vector<Scope*>&           partition() { return m_partition; }
        const std::vector<Scope*>&     partition() const { return m_partition; }
        void                           partition_add(Scope *);
        Scope*                         partition_at(size_t pos) { return m_partition.at(pos); };
        bool                           is_orphan() const { return m_parent == nullptr; }
        size_t                         depth() const { return m_depth; };
        static void                    change_node_scope(Node *node, Scope *desired_scope);
        static Scope*                  lowest_common_ancestor(const std::vector<Scope*>& scopes);
        static Scope*                  lowest_common_ancestor(Scope* s1, Scope* s2);
        static std::set<Scope*>&       get_descendent(std::set<Scope*>& out, Scope* scope, ScopeFlags flags = ScopeFlags_INCLUDE_SELF) { return get_descendent_ex(out, scope, -1, flags); }
        static std::set<Scope*>&       get_descendent_ex(std::set<Scope*>& out, Scope* scope, size_t level_max = -1, ScopeFlags = ScopeFlags_INCLUDE_SELF);

    private:
        VariableNode*                  find_var_ex(const std::string& identifier, ScopeFlags);
        void                           child_push_back_ex(Node*, ScopeFlags);
        void                           child_erase_ex(Node*, ScopeFlags);
        std::vector<Node*>&            leaves_ex(std::vector<Node*>& out);
        void                           node_register_add(Node *node, ScopeFlags flags);
        bool                           node_register_remove(Node *node, ScopeFlags flags);

        ScopeView*                     m_view = nullptr;
        std::set<Node*>                m_related;
        std::set<VariableNode*>        m_variable;
        std::vector<Node*>             m_child;
        std::vector<Scope*>            m_partition;
        Scope*                         m_parent = nullptr;
        size_t                         m_depth = 0;
    };
}
