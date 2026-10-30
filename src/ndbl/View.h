#pragma once

#include <algorithm>
#include <functional>
#include <vector>

#include "bdc/Types.hpp"
#include "Asserts.h"
#include "geometry/Rect.h"
#include "geometry/Space.h"
#include "Hash.h"
#include "Signals.h"

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

    Rect view_bounding_rect(const std::vector<View>&, Space space = WORLD_SPACE);

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
        Signal<void(View_Selection_Event_Type, View)> signal_change;


        Iterator        begin()        { return items.begin(); }
        Iterator        end()          { return items.end(); }
        Const_Iterator  cbegin() const { return items.cbegin(); }
        Const_Iterator  cend() const   { return items.cend(); }
        View&           front()        { return items.front(); }
        View&           back()         { return items.back(); }
        bool            empty() const  { return items.empty(); }        
    };

    void                view_selection_clear(View_Selection*);
    bool                view_selection_contains(const View_Selection*, const View& );
    void                view_selection_add(View_Selection*, const View& );
    void                view_selection_add(View_Selection*, const std::vector<View>&);
    bool                view_selection_remove(View_Selection*, const View&);
    bool                view_selection_contains(const View_Selection*, View_Type);
    size_t              view_selection_count(const View_Selection*, View_Type);
    View                view_selection_first_of(const View_Selection*, View_Type);
    std::vector<View>*  view_selection_collect(std::vector<View>* out, const View_Selection* in, View_Type type);
    
    template<class Iterator>
    size_t view_selection_add(View_Selection* selection, Iterator begin, Iterator end )
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
}

// required to compare Variant<..., Node_Slot_Link_View>
inline bool operator==(const ndbl::Node_Slot_Link_View& a, const ndbl::Node_Slot_Link_View& b) 
{
    return a.tail == b.tail && a.head == b.head;
}

// Custom hash provided to work in std::hash<std::Variant<Node_Slot_Link_View, ...>>
template<>
struct std::hash<ndbl::Node_Slot_Link_View>
{
    std::size_t operator()(const ndbl::Node_Slot_Link_View& edge) const noexcept
    { return ndbl::Hash::hash(edge); }
};