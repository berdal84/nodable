#pragma once

#include "gui/geometry/Vec2.h"
#include <cassert>
#include <cstddef>
#include <cstdio>
#include <stack>
#include <vector>

namespace tools
{
    typedef int Element_Type;
    enum Element_Type_
    {
        Element_Type_Element      = 0,
        Element_Type_Flex_Element = 1,
    };

    struct Style
    {
        enum Axis
        {
            Row,
            Column
        };

        float   pad_left    = 0.f;
        float   pad_top     = 0.f;
        float   pad_right   = 0.f;
        float   pad_bottom  = 0.f;          
        float   gap         = 0.f;
        Axis    main_axis   = Row;
    };    

    struct Element
    {

        Element_Type    type        = 0;
        float           x           = 0;
        float           y           = 0;
        float           width       = 0; // top-left corner is origin
        float           height      = 0;

        
        Element*        prev        = nullptr;
        Element*        next        = nullptr;
        
        // first/last are children (as double linked list)
        Element*        first_child = nullptr;
        Element*        last_child  = nullptr;

        Style*          style       = nullptr;
        void*           userdata    = nullptr;
    };

    struct Layout_State
    {
        float                  cursor_x     = 0.f;
        float                  cursor_y     = 0.f;
        std::vector<Style>     styles;
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
            printf("Element %lu: %f x %f px.\n", i, curr_elem->width, curr_elem->height );
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
        layout()->cursor_x = 0;
        layout()->cursor_y = 0;

        // We want to limit to X containers and Y elements
        layout()->elements.reserve(4096);
        layout()->styles.reserve(4096);
    }

    inline void layout_deinit()
    {
        layout()->elements.clear();
        layout()->styles.clear();

        while(!layout()->stack.empty())   
        {
            layout()->stack.pop();
        }
    }

    inline Element* layout_new_elem(const Element& elem)
    {
        Element* result = &layout()->elements.emplace_back(elem);
        result->style   = &layout()->styles.emplace_back();
        return result;
    }

    inline Element* layout_new_flex_elem()
    {
        Element elem;
        elem.type = Element_Type_Flex_Element;
        return layout_new_elem(elem);
    }

    inline void layout_begin()
    {
        assert(layout()->stack.size() == 0 && "LAYOUT_BEGIN/END mismatch!");

        Element* root_elem = layout_new_flex_elem();
        layout()->stack.push( root_elem );
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
    
    inline void layout_begin_flex(Style::Axis axis)
    {
        assert(layout()->elements.size() < layout()->elements.capacity() && "Buffer overflow!");
        
        Element* parent = layout()->stack.top();

        Element* elem = layout_new_flex_elem();
        elem->style->main_axis = axis;

        elem->x    = layout()->cursor_x;
        elem->y    = layout()->cursor_y;

        layout()->stack.push(elem);
        elem_push_back(parent, elem);
    }

    inline void layout_compute_sizes_and_positions()
    {
        // I compute sizes and positions from last to first Element
        // This way I am sure I start from leaf and end with root.

        for(auto it = layout()->elements.rbegin(); it != layout()->elements.rend(); ++it)
        {
            Element& parent = *it;

            if (parent.type != Element_Type_Flex_Element)   continue;
            if (elem_is_empty(&parent))            continue;

            Element* first_elem = parent.first_child;     

            size_t  elem_count              = 0;
            float   total_content_width     = 0.f;
            float   total_content_height    = 0.f;

            Element* elem  = first_elem;

            while(elem != nullptr)
            {

                if( elem == first_elem)
                {
                    elem->x = parent.x + parent.style->pad_left;
                    elem->y = parent.y + parent.style->pad_top;
                }
                else
                {
                    assert(elem->prev != nullptr && "Non first must have a previous element!");

                    if(parent.style->main_axis == Style::Row)
                    {
                        elem->x = elem->prev->x + elem->prev->width + parent.style->gap;
                        elem->y = elem->prev->y;
                    }
                    else
                    {
                        elem->x = elem->prev->x;
                        elem->y = elem->prev->y + elem->prev->height + parent.style->gap;
                    }
                }

                total_content_width  += elem->width;
                total_content_height += elem->height;

                elem = elem->next;
                elem_count++;
            }

            if( parent.style->main_axis == Style::Row )
            {
                parent.width    = parent.style->pad_left + parent.style->pad_right
                                + total_content_width
                                + parent.style->gap * ( elem_count - 1);
            }
            else
            {
                parent.height   = parent.style->pad_top + parent.style->pad_bottom
                                + total_content_height
                                + parent.style->gap * ( elem_count - 1);
            }
        }
    }

    inline void layout_set_cursor(Vec2 pos)
    {
        layout()->cursor_x = pos.x;
        layout()->cursor_y = pos.y;
    }

    inline void layout_set_gap(float gap)
    {
        Element* elem = layout()->stack.top();
        assert(elem->type == Element_Type_Flex_Element);
        elem->style->gap = gap;
    }


    inline void layout_push(const Element& elem)
    {
        assert(layout()->elements.size() < layout()->elements.capacity() && "Buffer overflow!");
        
        Element* parent = layout()->stack.top();
        assert(parent->type == Element_Type_Flex_Element);

        Element* new_element = layout_new_elem(elem);
        elem_push_back(parent, new_element);
    }

    inline void layout_set_left_padding(float size)
    {
        Element* elem = layout()->stack.top();
        elem->style->pad_left = size;
    }
}
