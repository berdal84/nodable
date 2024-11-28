#pragma once
#include "NodeView.h"
#include "ScopeView.h"
#include <vector>

namespace ndbl
{
    struct Selection
    {
        void append(NodeView* view)
        {
            _node.push_back(view);
            view->state().selected = true;
        }

        void append(ScopeView* view)
        {
            _scope.push_back(view);
            view->state().selected = true;
        }

        void remove(NodeView* view)
        {
            view->state().selected = false;
            _node.erase( std::find(_node.begin(), _node.end(), view) );
        }

        void remove(ScopeView* view)
        {
            view->state().selected = false;
            _scope.erase( std::find(_scope.begin(), _scope.end(), view) );
        }

        bool empty() const
        {
            return _node.empty() && _scope.empty();
        }

        bool contains(NodeView* view) const
        {
            return std::find(_node.begin(), _node.end(), view) != _node.end();
        }

        void clear()
        {
            for( NodeView* view : _node )
            {
                view->state().selected = false;
            }

            for( ScopeView* view : _scope  )
            {
                view->state().selected = false;
            }

            _node.clear();
            _scope.clear();
        }

        const std::vector<NodeView*>& node() const { return _node; };
        const std::vector<ScopeView*>& scope() const { return _scope; };
    private:
        std::vector<NodeView*>  _node;
        std::vector<ScopeView*> _scope;
    };
}