#pragma once

#include "core/Asserts.h"
#include "gui/geometry/Vec2.h"
#include <cassert>
#include <cstddef>
#include <cstdio>
#include <stack>
#include <vector>

namespace tools
{
    struct Padding
    {
        float left    = 0.f;
        float top     = 0.f;
        float right   = 0.f;
        float bottom  = 0.f;  
    };

    struct Sizing
    {
        union {
            struct {
                float width  = 0.f;
                float height = 0.f;
            };
            Vec2 vec;
        };
    };

    enum Axis
    {
        AXIS_NONE = 0,
        AXIS_LEFT_TO_RIGHT,
        AXIS_TOP_TO_BOTTOM
    };

    struct Container_Config
    {        
        float   gap         = 0.f;
        Axis    main_axis   = AXIS_LEFT_TO_RIGHT;
    };    

    typedef int Position_Mode;
    enum Relative_To_ {
        Position_Mode_RELATIVE = 0,
        Position_Mode_FLOATING = 1,
    };

    struct Element
    {
        typedef int Type;
        enum Type_
        {
            Type_LEAF      = 0,
            Type_CONTAINER = 1,
        };

        Type                type        = 0;
        Vec2                position    = {};
        Position_Mode       position_mode = 0;
        Padding             padding     = {};
        Sizing              dimension   = {-1, -1}; 
        Container_Config    container   = {};
        Element*            prev        = nullptr;
        Element*            next        = nullptr;
        Element*            first_child = nullptr; // first/last are children (as double linked list)
        Element*            last_child  = nullptr;
        u8_t                depth       = 0;
        void*               userdata    = nullptr;
    };

    inline Vec2 element_rect_min(const Element* el)
    {
        return el->position + Vec2{ el->padding.left, el->padding.top };
    }
    
    inline Vec2 element_rect_max(const Element* el)
    {
        return el->position + el->dimension.vec - Vec2{ el->padding.right, el->padding.bottom };
    }

    struct Layout_State
    {
        bool                   should_be_cleared = false;
        std::vector<Element>   elements;
        std::stack<Element*>   stack;
    };

    static Layout_State layout_state;

    inline Layout_State* layout() { return &layout_state; }


    inline void element_link(Element* a, Element* b)
    {
        assert(a != nullptr);
        assert(b != nullptr);

        b->prev = a;
        a->next = b;
    }

    inline void elem_push_back(Element* parent, Element* new_child)
    {
        if( parent->first_child == nullptr )
        {
            parent->first_child = new_child;
        }
        else
        {
            element_link(parent->last_child, new_child);            
        }
        parent->last_child = new_child;
    }

    inline void elem_print(Element* elem)
    {
        printf("State elements:\n");
        Element* curr_elem = elem->first_child;
        size_t   i    = 0;
        while(curr_elem != nullptr)
        {
            printf("Element %lu: %f x %f px.\n", i, curr_elem->dimension.width, curr_elem->dimension.height );
            ++i;
        }
        printf("--\n");
    }

    inline bool elem_is_empty(Element* elem)
    {
        return elem->first_child == nullptr;
    }

    inline void layout_init()
    {
        layout()->elements.reserve(4096);
    }

    inline void layout_deinit()
    {
        layout()->elements.clear();

        while(!layout()->stack.empty())   
        {
            layout()->stack.pop();
        }
    }

    inline void layout_begin_frame()
    {
        if( layout()->should_be_cleared )
        {
            layout_deinit();
        }
        layout_init();
    }

    inline void layout_end_frame()
    {
        layout()->should_be_cleared = true;
    }

    inline Element* layout_new_elem(const Element& elem)
    {
        return &layout()->elements.emplace_back(elem);
    }

    inline Element* layout_new_container(Axis axis)
    {
        Element elem;
        elem.type = Element::Type_CONTAINER;
        elem.container.main_axis = axis;
        return layout_new_elem(elem);
    }

    inline void layout_begin()
    {
        Element* elem = layout_new_container(AXIS_NONE);
        layout()->stack.push( elem );
    };

    inline void layout_end()
    {
        assert(layout()->stack.size() > 0 && "LAYOUT_BEGIN/END mismatch!");
        auto* current = layout()->stack.top();
        layout()->stack.pop();     
    };

    inline std::vector<Element>& layout_elements()
    {
        return layout()->elements;
    }
    
    inline Element* layout_current_element()
    {
        return &layout()->elements.back();
    }

    inline void layout__begin(Axis axis)
    {
        assert(layout()->elements.size() < layout()->elements.capacity() && "Buffer overflow!");
        
        Element* parent = layout()->stack.top();
        Element* elem  = layout_new_container(axis);

        layout()->stack.push(elem);
        elem_push_back(parent, elem);
    }

    inline void layout_begin_row()
    {
        return layout__begin(AXIS_LEFT_TO_RIGHT);
    }

    inline void layout_begin_column()
    {
        return layout__begin(AXIS_TOP_TO_BOTTOM);
    }

    inline void layout_compute_sizes_and_positions()
    {
        // I compute sizes and positions from last to first Element
        // This way I am sure I start from leaf and end with root.

        // PASS 1: compute sizes
        for(auto it = layout()->elements.rbegin(); it != layout()->elements.rend(); ++it)
        {
            Element& parent = *it;

            if ( parent.type != Element::Type_CONTAINER || elem_is_empty(&parent) )
            {
                continue;
            }

            Sizing   content;
            size_t   elem_count = 0;
            Element* elem       = parent.first_child;

            while(elem != nullptr)
            {
                // TODO:
                // - handle 3 dimensions (outer-box, box, content-box) for padding/margins in children
                // - handle when a node is pinned, row/column must be reset at pinned node
                //
                switch(parent.container.main_axis)
                {
                    case AXIS_LEFT_TO_RIGHT:
                    {
                        content.width += elem->dimension.width; 
                        content.height = std::max( content.height, elem->dimension.height );
                        break;
                    }

                    case AXIS_TOP_TO_BOTTOM:
                    {
                        content.width   = std::max( content.width, elem->dimension.width );
                        content.height += elem->dimension.height;
                        break;
                    }

                    case AXIS_NONE:
                    {
                        content.width  = std::max( content.width, elem->position.x + elem->dimension.width );
                        content.height = std::max( content.height, elem->position.y + elem->dimension.height ); 
                        break;
                    }
                }
                
                elem = elem->next;
                elem_count++;
            }

            switch ( parent.container.main_axis)
            {
                case AXIS_LEFT_TO_RIGHT:
                {
                    parent.dimension.width  = parent.padding.left + content.width  + (elem_count-1) * parent.container.gap + parent.padding.right;
                    parent.dimension.height = parent.padding.top  + content.height + parent.padding.bottom;
                    break;
                }

                case AXIS_TOP_TO_BOTTOM:
                {
                    parent.dimension.width  = parent.padding.left + content.width  + parent.padding.right;
                    parent.dimension.height = parent.padding.top  + content.height + (elem_count-1) * parent.container.gap + parent.padding.bottom ;
                    break;
                }

                case AXIS_NONE:
                {
                    parent.dimension.width  = parent.padding.left + content.width  + parent.padding.right;
                    parent.dimension.height = parent.padding.top  + content.height + parent.padding.bottom ;
                    break;
                }

                default:
                {
                    TOOLS_UNREACHABLE("Unexpected Axis: %i", parent.container.main_axis );
                }
            }
        }

        // PASS 2: compute positions
        //         Before this step, positions are relative to parent (by default) or world-space (when Position_Mode_STATIC is set).
        for(auto it = layout()->elements.begin(); it != layout()->elements.end(); ++it)
        {
            Element& element = *it;

            if (element.type != Element::Type_CONTAINER)
            {
                // A non-container should already have its position at this step.
                // Indeed, we update its container first, and each container set its children's positions.
                continue;
            }

            if (elem_is_empty(&element))
            {
                // An empty container can be considered as a LEAF, nothing to do here.
                continue;
            }

            assert(element.dimension.width != -1);
            assert(element.dimension.height != -1);

            Vec2 cursor = element.position;

            // Set initial cursor position
            switch ( element.container.main_axis)
            {
                case AXIS_LEFT_TO_RIGHT:
                {
                    cursor.x += element.padding.left;
                    cursor.y += element.padding.top;
                    break;
                }

                case AXIS_TOP_TO_BOTTOM:
                {
                    cursor.x += element.padding.left;
                    cursor.y += element.padding.top;
                    break;
                }

                case AXIS_NONE:
                {
                    break;
                }

                default:
                {
                    TOOLS_UNREACHABLE("Unexpected Axis: %i", element.container.main_axis );
                }
            }
            

            // Set each child element's position and depth
            Element* child = element.first_child;
            while(child != nullptr)
            {  
                child->depth = element.depth + 1;
                
                if( child->position_mode != Position_Mode_FLOATING )
                {
                    child->position = cursor;
                }

                switch ( element.container.main_axis )
                {                
                    case AXIS_LEFT_TO_RIGHT:
                    {
                        if( child != element.first_child->first_child)
                        {
                            cursor.x += element.container.gap;
                        }
                        cursor.x += child->dimension.width + child->padding.left;
                        break;
                    }

                    case AXIS_NONE:
                    case AXIS_TOP_TO_BOTTOM:
                    {
                        if( child != element.first_child->first_child)
                        {
                            cursor.y += element.container.gap;
                        }
                        cursor.y += child->dimension.height;
                        break;
                    }

                    default:
                    {
                        TOOLS_UNREACHABLE("Unexpected Container_Config: %i", element.container.main_axis );
                    }
                }

                child = child->next;
            }     
        }
    }

    inline void layout_set_gap(float gap)
    {
        Element* elem = layout()->stack.top();
        assert(elem->type == Element::Type_CONTAINER);
        elem->container.gap = gap;
    }

    inline void layout_push(const Element& elem)
    {
        assert(layout()->elements.size() < layout()->elements.capacity() && "Buffer overflow!");
        
        Element* parent = layout()->stack.top();
        assert(parent->type == Element::Type_CONTAINER);

        Element* new_element = layout_new_elem(elem);
        elem_push_back(parent, new_element);
    }

    inline void layout_set_floating()
    {
        Element* elem = layout_current_element();
        elem->position_mode = Position_Mode_FLOATING;
    }

    inline void layout_set_floating_at_position(Vec2 position)
    {
        Element* elem = layout_current_element();
        elem->position      = position;
        elem->position_mode = Position_Mode_FLOATING;
    }

    inline void layout_append_element(float width, float height, void* userdata)
    {
        Element elem;
        elem.dimension.width    = width;
        elem.dimension.height   = height;
        elem.userdata           = userdata;
        layout_push(elem);
    }

    inline void layout_push_vspace(float size)
    {
        Element elem;
        elem.dimension.height = size;
        elem.dimension.width  = 1.f;
        layout_push(elem);
    }

    inline void layout_push_hspace(float size)
    {
        Element elem;
        elem.dimension.height = 1.f;
        elem.dimension.width  = size;
        layout_push(elem);
    }

    inline void layout_set_padding(float left, float top, float right, float bottom)
    {
        Element* elem = layout_current_element();
        elem->padding = { left, top, right, bottom };
    }
}
