#include "View.h"

#include "Node_View.h"

namespace ndbl
{
Rect view_bounding_rect( const std::vector<View>& views, Space space )
{        
    using namespace ndbl;

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

void view_selection_clear(View_Selection* selection)
{
    for(const View& elem : selection->items )
    {
        selection->signal_change.emit( View_Selection_Event_Type_REMOVE, elem );
    }
    selection->items.clear();
    selection->count_by_type.clear();
}

bool view_selection_contains(const View_Selection* selection, const View& elem )
{
    for(auto& each : selection->items)
        if( each == elem)
            return true;
    return false;
}

void view_selection_add(View_Selection* selection, const View& elem )
{
    selection->items.push_back(elem);
    selection->count_by_type[elem.type]++;
    selection->signal_change.emit( View_Selection_Event_Type_APPEND, elem );
}

void view_selection_add(View_Selection* selection, const std::vector<View>& views)
{
    view_selection_add(selection, views.begin(), views.end());
}

bool view_selection_remove(View_Selection* selection, const View& elem)
{
    auto found = std::find( selection->items.cbegin(), selection->items.cend(), elem) != selection->items.cend();
    if( found )
    {
        selection->count_by_type[elem.type]--;
        selection->signal_change.emit( View_Selection_Event_Type_APPEND, elem );
    }
    return found;
}

bool view_selection_contains(const View_Selection* selection, View_Type type) // O(1), read from cache.
{
    return selection->count_by_type.contains(type);
}

size_t view_selection_count(const View_Selection* selection, View_Type type) // O(1), read from cache.
{
    if ( selection->count_by_type.contains(type ) )
    {
        return selection->count_by_type.at(type );
    }
    return 0;
}

View view_selection_first_of(const View_Selection* selection, View_Type type) // O(n), I suggest you to use contains() once first
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

std::vector<View>* view_selection_collect(std::vector<View>* out, const View_Selection* selection, View_Type type) // O(n), do only a single allocation when necessary
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

} // namespace ndbl