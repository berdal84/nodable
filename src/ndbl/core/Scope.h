#pragma once

#include <vector>
#include <set>
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

    struct Scope
    {
        bdc::String                     name = "Scope";
        std::set<Node*>                 children;
        std::set<Node*>                 variables;
        mutable std::vector<Node*>      _cached_backbone;
        Token                           token_begin;
        Token                           token_end;
        Node*                           head    = nullptr; // backbone's start
        Scope*                          parent  = nullptr;
        Scope_View*                     view    = nullptr;
        mutable size_t                  _cached_depth;
        mutable bool                    _cached_backbone_dirty;
        mutable bool                    _cached_depth_dirty;
        Node*                           node = nullptr;
    };

    void                                scope_init(Scope*);
    void                                scope_deinit(Scope*);
    std::vector<Node*>                  scope_get_leaves(Scope*);
    std::vector<Node*>&                 scope_get_leaves_ex(std::vector<Node*>& out, Scope*);
    size_t                              scope_get_depth(const Scope* scope);
    void                                scope_append(Scope*, Node*);
    void                                scope_remove(Scope*, Node*);
    bool                                scope_contains(const Scope*, Node*);
    bool                                scope_is_empty(const Scope*, Scope_Flags = Scope_Flag_NONE);
    bool                                scope_is_orphan(const Scope* scope);
    Node*                               scope_find_variable(Scope*, const bdc::String& identifier, Scope_Flags = Scope_Flag_RECURSE_PARENT_SCOPES);
    void                                scope_reset_head(Scope*, Node* node = nullptr);
    const std::vector<Node*>&           scope_get_backbone(const Scope* scope); // backbone is a vector of nodes starting from the scope's head including all flow_outputs in this scope (recursively)
    void                                scope_reset_parent(Scope* scope, Scope* new_parent = nullptr);
    Scope*                              scope_find_lowest_common_ancestor(const std::set<Scope*>& scopes);
    Scope*                              scope_lowest_common_ancestor(Scope* s1, Scope* s2);
    std::set<Scope*>&                   scope_get_descendent_ex(std::set<Scope*>& out, Scope* scope, size_t level_max = -1, Scope_Flags = Scope_Flag_INCLUDE_SELF);
    std::set<Scope*>&                   scope_get_descendent(std::set<Scope*>& out, Scope* scope, Scope_Flags flags = Scope_Flag_INCLUDE_SELF);
}
