#pragma once
#include "NodeView.h"
#include "ScopeView.h"
#include <vector>

namespace ndbl
{
    struct Selection
    {
        Selection()
        {}

        Selection(ScopeView* view)
        { add(view); }

        Selection(NodeView* view)
        { add(view); }

        Selection(const std::vector<NodeView*>& views)
        {
            for ( auto view : views )
                add( view );
        }

        void remove_all()
        {
            for( NodeView* view : node )
            {
                view->state().selected = false;
            }

            for( ScopeView* view : scope  )
            {
                view->state().selected = false;
            }

            node.clear();
            scope.clear();
        }

        void add(const Selection& selection)
        {
            for ( auto each : selection.node ) add( each );
            for ( auto each : selection.scope ) add( each );
        }

        void add(NodeView* view)
        {
            node.push_back(view);
            view->state().selected = true;
        }

        void add(ScopeView* view)
        {
            scope.push_back(view);
            view->state().selected = true;
        }

        void remove(NodeView* view)
        {
            view->state().selected = false;
            node.erase( std::find(node.begin(), node.end(), view) );
        }

        void remove(ScopeView* view)
        {
            view->state().selected = false;
            scope.erase( std::find(scope.begin(), scope.end(), view) );
        }

        [[nodiscard]] bool empty() const
        {
            return node.empty() && scope.empty();
        }

        bool has(NodeView* view) const
        {
            return std::find(node.begin(), node.end(), view) != node.end();
        }

        std::vector<NodeView*>  node;
        std::vector<ScopeView*> scope;

    };
}