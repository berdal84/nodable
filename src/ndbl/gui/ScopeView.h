#pragma once
#include "ndbl/core/NodeComponent.h"
#include "tools/gui/geometry/Rect.h"

namespace ndbl
{
    // forward decl.
    class Scope;

    typedef int ScopeViewFlags;
    enum ScopeViewFlags_
    {
        ScopeViewFlags_NONE    = 0,
        ScopeViewFlags_RECURSE = 1
    };

    class ScopeView : public NodeComponent
    {
    public:
        typedef tools::Rect Rect;

        ScopeView(Scope* scope);
        void         update(float dt, ScopeViewFlags flags = ScopeViewFlags_NONE );
        void         draw(float dt);
        Scope*       scope() const { return m_scope; }
        const Rect&  rect() const { return m_rect; }

    private:
        Rect        m_rect;
        Scope*      m_scope;
    };
}