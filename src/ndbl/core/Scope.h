#pragma once

#include <vector>
#include <set>
#include "tools/core/Component.h"
#include "Token.h"
#include "Node.h"

namespace ndbl
{
    // forward decl.
    class Node;
    class Scope_View;

    typedef int Scope_Flags;
    enum Scope_Flag_
    {
        Scope_Flag_NONE                    = 0,
        Scope_Flag_RECURSE_PARENT_SCOPES   = 1 << 0,
        Scope_Flag_RECURSE_CHILD_PARTITION = 1 << 1,
        Scope_Flag_INCLUDE_SELF            = 1 << 2,
    };

    class Scope : public tools::Component<Node>
    {
//== Data ==============================================================================================================
    public:
        Token                           token_begin = {Token_Type::ignore};
        Token                           token_end   = {Token_Type::ignore};
    private:        
        Scope_View*                     m_view = nullptr;
        std::set<Node*>                 m_children;
        Node*                           m_head = nullptr; // backbone's start
        std::set<Node*>                 m_variables;
        std::vector<Scope*>             m_partition;
        Scope*                          m_parent = nullptr;
        mutable std::vector<Node*>      m_cached_backbone;
        mutable bool                    m_cached_backbone_dirty = true;
        mutable size_t                  m_cached_depth = 0;
        mutable bool                    m_cached_depth_dirty = true;
//== Methods ===========================================================================================================
    public:
        Scope();
        ~Scope() override;

        void                        append(Node*);
        void                        remove(Node*);
        bool                        contains(Node* node) const;
        Scope*                      parent() { return m_parent; }
        const Scope*                parent() const { return m_parent; }
        std::vector<Node*>          leaves();
        bool                        empty(Scope_Flags = Scope_Flag_NONE) const;
        Node*                       find_variable(const std::string& identifier, Scope_Flags = Scope_Flag_RECURSE_PARENT_SCOPES);
        Scope_View*                 view() const { return m_view; } // TODO: remove this, use components
        void                        set_view(Scope_View* view) { m_view = view; } // TODO: remove this, use components
        const std::set<Node*>&      variables()const { return m_variables; };
        const std::set<Node*>&      children() const { return m_children; }
        Node*                       head() const { return m_head; }
        void                        reset_head(Node* node = nullptr);
        const std::vector<Node*>&   backbone() const { _update_backbone_cache(); return m_cached_backbone; } // backbone is a vector of nodes starting from the scope's head including all flow_outputs in this scope (recursively)
        void                        reset_parent(Scope* new_parent = nullptr);
        bool                        is_orphan() const { return m_parent == nullptr; }
        size_t                      depth() const { _update_depth_cache(); return m_cached_depth; };
        Node*                       node() const { return entity; }; // alias for entity
        static Scope*               lowest_common_ancestor(const std::set<Scope*>& scopes);
        static Scope*               lowest_common_ancestor(Scope* s1, Scope* s2);
        static std::set<Scope*>&    get_descendent(std::set<Scope*>& out, Scope* scope, Scope_Flags flags = Scope_Flag_INCLUDE_SELF) { return get_descendent_ex(out, scope, -1, flags); }
        static std::set<Scope*>&    get_descendent_ex(std::set<Scope*>& out, Scope* scope, size_t level_max = -1, Scope_Flags = Scope_Flag_INCLUDE_SELF);
    private:
        void                        _on_shutdown();
        std::vector<Node*>&         _leaves_ex(std::vector<Node*>& out);
        void                        _update_backbone_cache() const;
        void                        _update_depth_cache() const;
        void                        _set_depth_cache_dirty() const;
    };
}
