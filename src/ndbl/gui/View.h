#pragma once
#include "core/Asserts.h"
#include "core/Signals.h"
#include "gui/Node_View.h"
#include "gui/geometry/Space.h"
#include "tools/core/Hash.h"
#include <algorithm>
#include <functional>
#include <vector>
#include "bdc/Types.hpp"
#include "tools/gui/geometry/Rect.h"

namespace ndbl
{
    // forward declarations
    struct Node_View;
    struct Scope_View;
    struct Node_Slot_View;

    struct Node_Slot_Link_View
    {
        Node_Slot_View* tail = nullptr;
        Node_Slot_View* head = nullptr;
    };
    
    typedef u8_t View_Type;
    enum View_Type_ : u8_t
    {
        View_Type_NULL = 0,
        View_Type_NODE,
        View_Type_SCOPE,
        View_Type_SLOT,
        View_Type_LINK
    };

    struct View
    {
        View_Type type = 0;

        union
        {
            struct {
                void* data1;
                void* data2;
            };
            Node_View*          nodeview;
            Scope_View*         scopeview;
            Node_Slot_View*     slotview;
            Node_Slot_Link_View linkview;        
        };


        View(): type(View_Type_NULL), data1(nullptr), data2(nullptr) {}
        View(Node_View* view): type(View_Type_NODE), data1(view), data2(nullptr) {}
        View(Scope_View* view): type(View_Type_SCOPE), data1(view), data2(nullptr)  {}
        View(Node_Slot_View* view): type(View_Type_SLOT), data1(view), data2(nullptr)  {}
        View(Node_Slot_Link_View view): type(View_Type_LINK), linkview(view) {}
        ~View() {};
    };

    static bool operator==(const View& a, const View& b)
    {
        return a.type == b.type && a.data1 == b.data1 && a.data2 == b.data2;
    }

    inline tools::Rect view_bounding_rect(
        const std::vector<View>& views,
        tools::Space space = tools::WORLD_SPACE
    )
    {        
        using namespace tools;

        // collect rectangles
        // note: we could save 1 allocation by computing the bbox of each rectangle instead of building this vector,
        //       but I prefer to keep responsibilities separated.
        std::vector<Rect> rect;
        rect.reserve(views.size());
        for (const View& view : views)
        {
            VERIFY( view.type == View_Type_NODE, "Must contain only Node_Views!");
            rect.emplace_back( nodeview_get_rect(view.nodeview, space) ) ;
        }
        // compute bbox
        return Rect::bounding_rect(rect);
    }

    typedef u8_t View_Selection_Event_Type;
    enum View_Selection_Event_Type_
    {
        View_Selection_Event_Type_APPEND = 1,
        View_Selection_Event_Type_REMOVE
    };

    struct View_Selection
    {
        using Iterator       = std::vector<View>::iterator;
        using Const_Iterator = std::vector<View>::const_iterator;


        std::vector<View>                     items;
        std::unordered_map<View_Type, size_t> count_by_type;

        Iterator        begin()        { return items.begin(); }
        Iterator        end()          { return items.end(); }
        Const_Iterator  cbegin() const { return items.cbegin(); }
        Const_Iterator  cend() const   { return items.cend(); }

        tools::Signal<void(View_Selection_Event_Type, View)> signal_change;

        View& front()
        { return items.front(); }

        View& back()
        { return items.back(); }

        bool empty() const
        { return items.empty(); }        
    };

    inline void view_selection_clear(View_Selection* selection)
    {
        for(const View& elem : selection->items )
        {
            selection->signal_change.emit( View_Selection_Event_Type_REMOVE, elem );
        }
        selection->items.clear();
        selection->count_by_type.clear();
    }

    inline bool view_selection_contains(const View_Selection* selection, const View& elem )
    {
        for(auto& each : selection->items)
            if( each == elem)
                return true;
        return false;
    }

    inline void view_selection_add(View_Selection* selection, const View& elem )
    {
        selection->items.push_back(elem);
        selection->count_by_type[elem.type]++;
        selection->signal_change.emit( View_Selection_Event_Type_APPEND, elem );
    }

    template<class Iterator> size_t view_selection_add(View_Selection* selection, Iterator begin, Iterator end )
    {
        size_t count = 0;

        auto it = begin;
        while( it != end)
        {
            view_selection_add( selection, *it );
            ++count;
            ++it;
        }

        return count;
    }

    inline void view_selection_add(View_Selection* selection, const std::vector<View>& views)
    {
        view_selection_add(selection, views.begin(), views.end());
    }

    inline bool view_selection_remove(View_Selection* selection, const View& elem)
    {
        auto found = std::find( selection->items.cbegin(), selection->items.cend(), elem) != selection->items.cend();
        if( found )
        {
            selection->count_by_type[elem.type]--;
            selection->signal_change.emit( View_Selection_Event_Type_APPEND, elem );
        }
        return found;
    }

    inline bool view_selection_contains(const View_Selection* selection, View_Type type) // O(1), read from cache.
    {
        return selection->count_by_type.contains(type);
    }

    inline size_t view_selection_count(const View_Selection* selection, View_Type type) // O(1), read from cache.
    {
        if ( selection->count_by_type.contains(type ) )
        {
            return selection->count_by_type.at(type );
        }
        return 0;
    }

    inline View view_selection_first_of(const View_Selection* selection, View_Type type) // O(n), I suggest you to use contains() once first
    {
        const size_t _count = view_selection_count(selection, type);
        if ( _count == 0 )
            return {};

        for ( const View& elem : selection->items )
            if ( elem.type == type )
                return elem;

        ASSERT(false); // unreachable case
        return {};
    }

    inline std::vector<View>* view_selection_collect(std::vector<View>* out, const View_Selection* selection, View_Type type) // O(n), do only a single allocation when necessary
    {
        const size_t _count = view_selection_count(selection, type);
        if ( _count == 0 )
            return {};

        out->reserve( _count ); // 1 allocation max :)

        // OPTIM: we could use a cache per type_index if necessary ( type_index => list<T*> )
        for ( const View& elem : selection->items )
            if ( elem.type == type )
                out->push_back( elem );

        return out;
    }
}

// required to compare tools::Variant<..., Node_Slot_Link_View>
inline bool operator==(const ndbl::Node_Slot_Link_View& a, const ndbl::Node_Slot_Link_View& b) 
{
    return a.tail == b.tail && a.head == b.head;
}

// Custom hash provided to work in std::hash<std::Variant<Node_Slot_Link_View, ...>>
template<>
struct std::hash<ndbl::Node_Slot_Link_View>
{
    std::size_t operator()(const ndbl::Node_Slot_Link_View& edge) const noexcept
    { return tools::Hash::hash(edge); }
};