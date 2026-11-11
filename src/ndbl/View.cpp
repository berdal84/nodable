#include "View.h"

#include "Node_View.h"

namespace ndbl
{

void viewselection_init(View_Selection& selection, Allocator* _allocator)
{
    array_resize(selection.count_by_type, selection.count_by_type.capacity);
}

void viewselection_release(View_Selection& selection)
{
    array_release(selection.items);
}

Rect view_bounding_rect( const bdc::Array<View>& views, Space space )
{        
    Rect result;
    for (const View& view : views)
    {
        NDBL_ASSERT( view.type == View_Type_NODE, "Must contain only Node_Views!");
        result = rect_bounding_rect(result, nodeview_get_rect(view.nodeview, space) ) ;
    }

    return result;
}

void viewselection_clear(View_Selection& selection)
{
    for(const View& elem : selection.items )
    {
        selection.signal_change.emit( View_Selection_Event_Type_REMOVE, elem );
    }
    
    array_clear(selection.items);

    for(auto& each : selection.count_by_type)
    {
        each = 0;
    }
}

bool viewselection_contains(const View_Selection& selection, const View& elem )
{
    for(auto& each : selection.items)
        if( each == elem)
            return true;
    return false;
}

void viewselection_add(View_Selection& selection, const View& elem )
{
    array_append(selection.items, elem);
    selection.count_by_type[elem.type]++;
    selection.signal_change.emit( View_Selection_Event_Type_APPEND, elem );
}

void viewselection_add(View_Selection& selection, const Array<View>& views )
{
    for(auto& view : views )
    {
        viewselection_add( selection, view );
    }
}

void viewselection_remove(View_Selection& selection, const View& elem)
{
    auto found = array_find(selection.items, elem);
    NDBL_ASSERT(found.ok);
    selection.count_by_type[elem.type]--;
    selection.signal_change.emit( View_Selection_Event_Type_APPEND, elem );
    array_remove_ordered(selection.items, found.at_pos);
}

bool viewselection_contains(const View_Selection& selection, View_Type type) // O(1), read from cache.
{
    return viewselection_count(selection, type) > 0;
}

size_t viewselection_count(const View_Selection& selection, View_Type type) // O(1), read from cache.
{
    return selection.count_by_type[type];
}

View viewselection_first_of(const View_Selection& selection, View_Type type) // O(n), I suggest you to use contains() once first
{
    const size_t _count = viewselection_count(selection, type);
    if ( _count == 0 )
        return {};

    for ( const View& elem : selection.items )
        if ( elem.type == type )
            return elem;

    NDBL_ASSERT(false); // unreachable case
    return {};
}

bdc::Resizable_Array<View>& viewselection_collect(bdc::Resizable_Array<View>& out, const View_Selection& selection, View_Type type) // O(n), do only a single allocation when necessary
{
    const size_t _count = viewselection_count(selection, type);
    if ( _count == 0 )
    {
        return out;
    }

    array_ensure_has_capacity(out, out.size + _count ); // 1 allocation max :)

    // OPTIM: we could use a cache per type_index if necessary ( type_index => list<T*> )
    for ( const View& elem : selection.items )
        if ( elem.type == type )
            array_append(out, elem );

    return out;
}

} // namespace ndbl