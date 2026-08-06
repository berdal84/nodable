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
        Scope_Flag_INCLUDE_SELF            = 1 << 2,
    };

    struct Scope : public tools::Component<Node>
    {
        Scope();
        ~Scope() override;

        std::set<Node*>                 children                = {};
        std::set<Node*>                 variables               = {};
        mutable std::vector<Node*>      _cached_backbone        = {};
        Token                           token_begin             = {Token_Type::ignore};
        Token                           token_end               = {Token_Type::ignore};
        Node*                           head                    = nullptr; // backbone's start
        Scope*                          parent                  = nullptr;
        Scope_View*                     view                    = nullptr;
        mutable size_t                  _cached_depth           = 0;
        mutable bool                    _cached_backbone_dirty  = true;
        mutable bool                    _cached_depth_dirty     = true;

        Node*                           node() const { return entity; }; // alias for entity
    };

    void                                _scope_update_backbone_cache(const Scope*);
    void                                _scope_update_depth_cache(const Scope*);
    void                                _scope_set_depth_cache_dirty(const Scope*);

    void                                scope_on_deinit(Scope*);
    std::vector<Node*>                  scope_get_leaves(Scope*);
    std::vector<Node*>&                 scope_get_leaves_ex(std::vector<Node*>& out, Scope*);
    inline size_t                       scope_get_depth(const Scope* scope) { _scope_update_depth_cache(scope); return scope->_cached_depth; };
    void                                scope_append(Scope*, Node*);
    void                                scope_remove(Scope*, Node*);
    bool                                scope_contains(const Scope*, Node*);
    bool                                scope_is_empty(const Scope*, Scope_Flags = Scope_Flag_NONE);
    inline bool                         scope_is_orphan(const Scope* scope) { return scope->parent == nullptr; }
    Node*                               scope_find_variable(Scope*, const std::string& identifier, Scope_Flags = Scope_Flag_RECURSE_PARENT_SCOPES);
    void                                scope_reset_head(Scope*, Node* node = nullptr);
    inline const std::vector<Node*>&    scope_get_backbone(const Scope* scope) { _scope_update_backbone_cache(scope); return scope->_cached_backbone; } // backbone is a vector of nodes starting from the scope's head including all flow_outputs in this scope (recursively)
    void                                scope_reset_parent(Scope* scope, Scope* new_parent = nullptr);
    Scope*                              scope_find_lowest_common_ancestor(const std::set<Scope*>& scopes);
    Scope*                              scope_lowest_common_ancestor(Scope* s1, Scope* s2);
    std::set<Scope*>&                   scope_get_descendent_ex(std::set<Scope*>& out, Scope* scope, size_t level_max = -1, Scope_Flags = Scope_Flag_INCLUDE_SELF);
    inline std::set<Scope*>&            scope_get_descendent(std::set<Scope*>& out, Scope* scope, Scope_Flags flags = Scope_Flag_INCLUDE_SELF) { return scope_get_descendent_ex(out, scope, -1, flags); }
}
