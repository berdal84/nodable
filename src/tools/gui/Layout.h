#pragma once

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
        float width  = 0.f;
        float height = 0.f;
    };

    struct Container_Config
    {
        enum Axis
        {
            AXIS_NONE = 0,
            AXIS_LEFT_TO_RIGHT,
            AXIS_TOP_TO_BOTTOM
        };
        
        float   gap         = 0.f;
        Axis    main_axis   = AXIS_LEFT_TO_RIGHT;
    };    

    typedef int Position_Mode;
    enum Relative_To_ {
        Position_Mode_RELATIVE = 0,
        Position_Mode_STATIC = 1,
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
        void*               userdata    = nullptr;
    };

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

    inline Element* layout_new_container(Container_Config::Axis axis)
    {
        Element elem;
        elem.type = Element::Type_CONTAINER;
        elem.container.main_axis = axis;
        return layout_new_elem(elem);
    }

    inline void layout_begin()
    {
        Element* elem = layout_new_container(Container_Config::AXIS_NONE);
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

    inline void layout_begin(Container_Config::Axis axis)
    {
        assert(layout()->elements.size() < layout()->elements.capacity() && "Buffer overflow!");
        
        Element* parent = layout()->stack.top();
        Element* elem  = layout_new_container(axis);

        layout()->stack.push(elem);
        elem_push_back(parent, elem);
    }

    inline void layout_compute_sizes_and_positions()
    {
        // I compute sizes and positions from last to first Element
        // This way I am sure I start from leaf and end with root.

        // PASS 1: compute sizes, and local positions
        for(auto it = layout()->elements.rbegin(); it != layout()->elements.rend(); ++it)
        {
            Element& parent = *it;

            if (parent.type != Element::Type_CONTAINER) continue;
            if (elem_is_empty(&parent))            continue;

            Sizing   content;
            size_t   elem_count = 0;
            Element* elem  = parent.first_child;

            while(elem != nullptr)
            {
                if(elem->position_mode == Position_Mode_STATIC)
                {
                    elem = elem->next;
                    continue;
                }

                switch(parent.container.main_axis)
                {
                    case Container_Config::AXIS_LEFT_TO_RIGHT:
                    {
                        content.width += elem->dimension.width;
                        content.height = std::max( content.height, elem->dimension.height );
                        break;
                    }

                    case Container_Config::AXIS_TOP_TO_BOTTOM:
                    {
                        content.width   = std::max( content.width, elem->dimension.width );
                        content.height += elem->dimension.height;
                        break;
                    }

                    case tools::Container_Config::AXIS_NONE:
                    {
                        content.width  = std::max( content.width, elem->position.x + elem->dimension.width );
                        content.height = std::max( content.height, elem->position.y + elem->dimension.height );
                        break;
                    }
                }
                
                elem = elem->next;
                elem_count++;
            }

            if( parent.container.main_axis == Container_Config::AXIS_LEFT_TO_RIGHT )
            {
                parent.dimension.width  = parent.padding.left + content.width  + elem_count * parent.container.gap + parent.padding.right;
                parent.dimension.height = parent.padding.top  + content.height + parent.padding.bottom;
            }
            else if( parent.container.main_axis == Container_Config::AXIS_TOP_TO_BOTTOM )
            {
                parent.dimension.width  = parent.padding.left + content.width  + parent.padding.right;
                parent.dimension.height = parent.padding.top  + content.height + elem_count * parent.container.gap + parent.padding.bottom ;
            }
            else
            {
                parent.dimension.width  = parent.padding.left + content.width  + parent.padding.right;
                parent.dimension.height = parent.padding.top  + content.height + parent.padding.bottom ;
            }
        }

        // PASS 2: compute positions to make sure any non-floating element has a world positions and not a relative one
        for(auto it = layout()->elements.begin(); it != layout()->elements.end(); ++it)
        {
            Element& element = *it;

            if (element.type != Element::Type_CONTAINER)  continue;
            if (elem_is_empty(&element))            continue;

            assert(element.dimension.width != -1);
            assert(element.dimension.height != -1);

            Vec2 cursor;
            cursor.x = element.padding.left;
            cursor.y = element.padding.top;

            Element* child = element.first_child;
            while(child != nullptr)
            {  
                if(child->position_mode == Position_Mode_STATIC)
                {
                    // no need to update position since it is already in world space
                    child = child->next;
                    continue;
                }
                
                if( element.container.main_axis == Container_Config::AXIS_LEFT_TO_RIGHT)
                {
                    child->position = element.position + cursor;
                    cursor.x += child->dimension.width + element.container.gap;
                }
                else if( element.container.main_axis == Container_Config::AXIS_TOP_TO_BOTTOM)
                {
                    child->position = element.position + cursor;
                    cursor.y += child->dimension.height + element.container.gap;
                }
                else if( element.container.main_axis == Container_Config::AXIS_NONE)
                {
                    child->position += element.position;
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

    inline void layout_pin_element()
    {
        Element* elem = layout_current_element();
        elem->position_mode = Position_Mode_STATIC;
    }

    inline void layout_pin_element_at_position(Vec2 position)
    {
        Element* elem = layout_current_element();
        elem->position      = position;
        elem->position_mode = Position_Mode_STATIC;
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

    inline void layout_set_padding(float left, float top = 0, float right = 0, float bottom = 0)
    {
        Element* elem = layout_current_element();
        elem->padding = { left, top, right, bottom };
    }
}
