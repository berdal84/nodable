#pragma once
#include "core/Asserts.h"
#include "core/Signals.h"
#include "gui/Node_View.h"
#include "gui/geometry/Space.h"
#include "tools/core/Hash.h"
#include <algorithm>
#include <functional>
#include "core/Types.h"
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

        void clear()
        {
            for(const View& elem : items )
            {
                signal_change.emit( View_Selection_Event_Type_REMOVE, elem );
            }
            items.clear();
            count_by_type.clear();
        }

        bool contains(const View& elem ) const
        {
            for(auto& each : items)
                if( each == elem)
                    return true;
            return false;
        }

        void push_back(const View& elem )
        {
            items.push_back(elem);
            count_by_type[elem.type]++;
            signal_change.emit( View_Selection_Event_Type_APPEND, elem );
        }

        template<typename View_Constructor_Arg_Type>
        void push_back(View_Constructor_Arg_Type arg)
        {
            return push_back(View{arg});
        }

        template<class Iterator> size_t push_back( Iterator begin, Iterator end )
        {
            size_t count = 0;

            auto it = begin;
            while( it != end)
            {
                push_back( *it );
                ++count;
                ++it;
            }

            return count;
        }

        bool remove(const View& elem)
        {
            auto found = std::find( items.cbegin(), items.cend(), elem) != items.cend();
            if( found )
            {
                count_by_type[elem.type]--;
                signal_change.emit( View_Selection_Event_Type_APPEND, elem );
            }
            return found;
        }

        bool contains(View_Type type) const // O(1), read from cache.
        {
            return count_by_type.contains(type);
        }

        size_t count(View_Type type) const // O(1), read from cache.
        {
            if ( count_by_type.contains(type ) )
            {
                return count_by_type.at(type );
            }
            return 0;
        }

        View first_of(View_Type type) const // O(n), I suggest you to use contains() once first
        {
            const size_t _count = count(type);
            if ( _count == 0 )
                return {};

            for ( const View& elem : items )
                if ( elem.type == type )
                    return elem;

            ASSERT(false); // unreachable case
            return {};
        }

        std::vector<View> collect(View_Type type) const // O(n), do only a single allocation when necessary
        {
            const size_t _count = count(type);
            if ( _count == 0 )
                return {};

            std::vector<View> result;
            result.reserve( _count ); // 1 allocation max :)

            // OPTIM: we could use a cache per type_index if necessary ( type_index => list<T*> )
            for ( const View& elem : items )
                if ( elem.type == type )
                    result.push_back( elem );

            return result;
        }
    };


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