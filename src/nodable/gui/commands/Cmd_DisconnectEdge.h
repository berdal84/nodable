#pragma once
#include "gui/Command.h"
#include "core/DirectedEdge.h"
#include "core/Graph.h"
#include "core/Property.h"

namespace ndbl
{
    class Cmd_DisconnectEdge : public AbstractCommand
    {
    public:
        Cmd_DisconnectEdge(DirectedEdge _edge)
        : m_src(_edge.prop.src)
        , m_dst(_edge.prop.dst)
        , m_graph(_edge.prop.src->get_owner()->get_parent_graph())
        , m_edge(_edge)
        {
            char str[200];
            snprintf(str
                    , sizeof(str)
                    , "DisconnectEdge\n"
                      " - src: \"%s\"\n"
                      " - dst: \"%s\"\n"
                    , m_src->get_name().c_str()
                    , m_dst->get_name().c_str() );
            m_description.append(str);
        }

        ~Cmd_DisconnectEdge() override = default;

        void execute() override
        {
            m_graph->disconnect(&m_edge);
        }

        void undo() override
        {
            m_edge = *m_graph->connect(m_src, m_dst);
        }

        const char* get_description() const override
        {
            return m_description.c_str();
        }

    private:
        std::string m_description;
        Property*   m_src;
        Property*   m_dst;
        DirectedEdge m_edge;
        Graph*  m_graph;
    };
}