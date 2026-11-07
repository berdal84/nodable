#pragma once

#include <algorithm>
#include <functional>

#include "bdc/Array.hpp"
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
        View_Type_LINK,

        View_Type_COUNT
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

    Rect view_bounding_rect(const bdc::Array<View>&, Space space = WORLD_SPACE);

    typedef u8_t View_Selection_Event_Type;
    enum View_Selection_Event_Type_
    {
        View_Selection_Event_Type_APPEND = 1,
        View_Selection_Event_Type_REMOVE
    };

    struct View_Selection
    {
        bdc::Resizable_Array<View>                    items;
        bdc::Inlined_Array<u32_t, View_Type_COUNT>    count_by_type = {0,0,0,0,0};
        Signal<void(View_Selection_Event_Type, View)> signal_change;       
    };

    void                viewselection_init(View_Selection&, bdc::Allocator* = bdc::allocator);
    void                viewselection_release(View_Selection&);
    void                viewselection_clear(View_Selection&);
    bool                viewselection_contains(const View_Selection&, const View& );
    void                viewselection_add(View_Selection&, const View& );
    void                viewselection_add(View_Selection&, const bdc::Array<View>& );
    bool                viewselection_remove(View_Selection&, const View&);
    bool                viewselection_contains(const View_Selection&, View_Type);
    size_t              viewselection_count(const View_Selection&, View_Type);
    View                viewselection_first_of(const View_Selection&, View_Type);
    bdc::Resizable_Array<View>& viewselection_collect(bdc::Resizable_Array<View>& out, const View_Selection& in, View_Type type);

}

inline bool operator==(const ndbl::Node_Slot_Link_View& a, const ndbl::Node_Slot_Link_View& b) 
{
    return a.tail == b.tail && a.head == b.head;
}

template<>
struct std::hash<ndbl::Node_Slot_Link_View>
{
    std::size_t operator()(const ndbl::Node_Slot_Link_View& edge) const noexcept
    { return ndbl::Hash::hash(edge); }
};